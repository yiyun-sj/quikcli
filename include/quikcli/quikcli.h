/* --------------------------------------------------------------------------------
 * QuikCli - an interactive command line interface builder
 *
 * MIT License
 *
 * Copyright (c) 2024 Yiyun Jia
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * --------------------------------------------------------------------------------
 */

#ifndef QC_QUIKCLI_H_
#define QC_QUIKCLI_H_

#include <cstring>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "constants.h"
#include "exception.h"
#include "quikcli/flag.h"

namespace quikcli {

class QuikCli {
public:
  /* Constructors & Destructors - No Copy Default Move */
  QuikCli(std::string name, std::string version)
      : name_{name}, version_{version} {
    add_flag(DefaultFlagNames::version, "print version information then exit",
             [&](std::vector<std::string> &) { version_flag_func(*this); })
        .set_param_count(0);
  }

  QuikCli(QuikCli &) = delete;
  QuikCli &operator=(QuikCli &) = delete;
  QuikCli(QuikCli &&) = default;
  QuikCli &operator=(QuikCli &&) = default;

  ~QuikCli() = default;

public:
  /* Getters & Setters */
  std::string name() const { return name_; }
  void set_name(std::string name) { name_ = name; }
  std::string version() const { return version_; }
  void set_version(std::string version) { version_ = version; }

  /* Configurations */
  Flag &add_flag(std::string name, std::string description) {
    check_no_dup_flag(name);
    flags_.emplace(name, Flag{name, description});
    return flags_.at(name);
  }
  Flag &add_flag(std::string name, std::string description,
                 callback_t callback) {
    check_no_dup_flag(name);
    flags_.emplace(name, Flag{name, description, std::move(callback)});
    return flags_.at(name);
  }
  template <class... ParamsT>
    requires(has_istream_operator<ParamsT> && ...)
  Flag &add_flag(std::string name, std::string description,
                 ParamsT &...params) {
    check_no_dup_flag(name);
    flags_.emplace(name, Flag{name, description, params...});
    return flags_.at(name);
  }

  /* Run-Time Functions */
  void parse_flags(int argc, char *argv[]) {
    try {
      std::vector<std::string> params;
      std::vector<Flag *> set_flags;
      Flag *current_flag = nullptr;
      for (int i = 1; i < argc; i++) {
        std::string argstr = argv[i];
        if (argstr.length() >= 2 && argstr[0] == '-') {
          Flag *next_flag;
          if (argstr[1] == '-') {
            std::string flag_name = argstr.substr(2);
            if (!flags_.contains(flag_name)) {
              throw Exception(ExceptionType::PARSER,
                              argstr + " is not a valid flag.");
            }
            next_flag = &flags_.at(flag_name);
          } else {
            // TODO: handle chained aliases (i.e. -abc)
            if (argstr.length() != 2 || !flag_aliases_.contains(argstr[1])) {
              throw Exception(ExceptionType::PARSER,
                              argstr + " is not a valid flag.");
            }
            next_flag = &flags_.at(flag_aliases_.at(argstr[1]));
          }
          if (next_flag->is_set()) {
            throw Exception(ExceptionType::PARSER,
                            argstr + " has already been set.");
          }
          if (current_flag) {
            current_flag->set(params);
            params.clear();
            set_flags.emplace_back(current_flag);
          }
          current_flag = next_flag;
        } else {
          if (!current_flag) {
            throw Exception(ExceptionType::PARSER,
                            "Expected flag but got \"" + argstr + "\".");
          }
          params.emplace_back(std::move(argstr));
        }
      }
      if (current_flag) {
        current_flag->set(params);
        set_flags.emplace_back(current_flag);
      }
      for (auto flag : set_flags) {
        flag->parse();
      }
    } catch (Exception &exception) {
      cleanup(exception);
      exit();
    }
  }

  void run() {
    if (!is_active_) {
      return;
    }
    std::string line;
    std::cout << name_ << "> ";
    while (std::cin >> line) {
      std::cout << line << '\n';
      if (!is_active_) {
        return;
      }
      std::cout << name_ << "> ";
    }
  }
  void exit() { is_active_ = false; }

private:
  /* Exception Handling */
  void cleanup(Exception exception) { std::cerr << exception.what() << '\n'; }

  /* Default Flags Callbacks */
  static void version_flag_func(QuikCli &cli) {
    std::cout << cli.name() << " version " << cli.version() << '\n';
    cli.exit();
  }

  /* Helpers */
  void check_no_dup_flag(std::string &name) {
    if (flags_.contains(name)) {
      throw Exception(ExceptionType::CONFIGURATION,
                      "flag " + name + " has been repeated.");
    }
  }

private:
  bool is_active_ = true;
  std::string name_;
  std::string version_;
  std::unordered_map<char, std::string> flag_aliases_;
  std::unordered_map<std::string, Flag> flags_;
};

} // namespace quikcli

#endif // QC_QUIKCLI_H_
