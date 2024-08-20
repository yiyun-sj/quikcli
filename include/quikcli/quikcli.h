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
#include <iomanip>
#include <ios>
#include <iostream>
#include <map>
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
    add_flag(DefaultFlagNames::version, DefaultFlagDescription::version,
             [&](std::vector<std::string> &) { default_version_func(*this); })
        .set_alias(DefaultFlagAliases::version)
        .set_param_count(0);
    add_flag(DefaultFlagNames::help, DefaultFlagDescription::help,
             [&](std::vector<std::string> &) { default_help_func(*this); })
        .set_alias(DefaultFlagAliases::help)
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
    check_dup_flag(name);
    flags_.emplace(name, Flag{name, description});
    return flags_.at(name);
  }
  Flag &add_flag(std::string name, std::string description,
                 callback_t callback) {
    check_dup_flag(name);
    flags_.emplace(name, Flag{name, description, std::move(callback)});
    return flags_.at(name);
  }
  template <class... ParamsT>
    requires(has_istream_operator<ParamsT> && ...)
  Flag &add_flag(std::string name, std::string description,
                 ParamsT &...params) {
    check_dup_flag(name);
    flags_.emplace(name, Flag{name, description, params...});
    return flags_.at(name);
  }

  /* Run-Time Functions */
  void parse_flags(int argc, char *argv[]) {
    std::unordered_map<char, std::string> aliases;
    for (const auto &[_, flag] : flags_) {
      if (flag.alias().has_value()) {
        aliases.emplace(flag.alias().value(), flag.name());
      }
    }
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
            if (argstr.length() != 2 || !aliases.contains(argstr[1])) {
              throw Exception(ExceptionType::PARSER,
                              argstr + " is not a valid flag.");
            }
            next_flag = &flags_.at(aliases.at(argstr[1]));
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
      std::cout << line << std::endl;
      if (!is_active_) {
        return;
      }
      std::cout << name_ << "> ";
    }
  }
  void exit() { is_active_ = false; }

private:
  /* Exception Handling */
  void cleanup(Exception exception) {
    std::cerr << exception.what() << std::endl;
  }

  /* Default Flags Callbacks */
  static void default_version_func(QuikCli &cli) {
    std::cout << cli.name() << " version " << cli.version() << std::endl;
    cli.exit();
  }
  static void default_help_func(QuikCli &cli) {
    constexpr int col_width = 18;
    constexpr char tab[] = "  ";
    std::cout << cli.name() << " version " << cli.version() << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    for (const auto &[_, flag] : cli.flags_) {
      std::string message = "--" + flag.name();
      if (flag.alias().has_value()) {
        message += ", -";
        message += flag.alias().value();
      }
      std::cout << tab << std::left << std::setw(col_width) << message;
      if (message.length() > col_width) {
        std::cout << std::endl << std::setw(col_width) << "";
      }
      std::cout << tab << flag.description() << std::endl;
    }
    cli.exit();
  }

  /* Helpers */
  void check_dup_flag(std::string &name) {
    if (flags_.contains(name)) {
      throw Exception(ExceptionType::CONFIGURATION,
                      "flag " + name + " has been repeated.");
    }
  }

private:
  bool is_active_ = true;
  std::string name_;
  std::string version_;
  std::map<std::string, Flag> flags_;
};

} // namespace quikcli

#endif // QC_QUIKCLI_H_
