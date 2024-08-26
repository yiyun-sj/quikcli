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

#ifndef QC_FLAG_H_
#define QC_FLAG_H_

#include <concepts>
#include <cstdint>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "quikcli/constants.h"
#include "quikcli/exception.h"

namespace quikcli {
class QuikCli;

using flag_callback_t = std::function<void(std::vector<std::string> &)>;

template <class Param>
concept has_istream_operator = requires(Param &param) {
  { std::declval<std::istream &>() >> param } -> std::same_as<std::istream>;
};

class Flag {
public:
  /* Constructors & Destructors - No Copy Default Move */
  Flag(std::string name, std::string description)
      : Flag(FlagParamSize::EMPTY, std::move(name), std::move(description),
             process_empty) {}
  Flag(std::string name, std::string description, flag_callback_t callback)
      : Flag(FlagParamSize::VARIADIC, std::move(name), std::move(description),
             std::move(callback)) {}
  template <class... ParamsT>
    requires(has_istream_operator<ParamsT> && ...)
  Flag(std::string name, std::string description, ParamsT &...params)
      : Flag(sizeof...(params), std::move(name), std::move(description),
             [&](std::vector<std::string> &inputs) {
               process_params(inputs, params...);
             }) {}

  Flag(Flag &) = delete;
  Flag &operator=(Flag &) = delete;
  Flag(Flag &&) = default;
  Flag &operator=(Flag &&) = default;

  ~Flag() = default;

public:
  /* Getters */
  bool is_set() const { return is_set_; }
  std::string name() const { return name_; }
  std::string description() const { return description_; }
  std::optional<char> alias() const { return alias_; }

  /* Configuration */
  Flag &set_immediate_parse() {
    immediate_parse_ = true;
    return *this;
  }
  Flag &set_param_count(uint32_t param_count) {
    param_count_ = param_count;
    return *this;
  }
  Flag &set_alias(char alias) {
    if (alias_set_.contains(alias)) {
      throw Exception(ExceptionType::CONFIGURATION,
                      "alias -" + std::to_string(alias) +
                          " has already been assigned.");
    }
    alias_.emplace(alias);
    return *this;
  }

  /* Parsing */
  void set(std::vector<std::string> params) {
    if (param_count_ != FlagParamSize::VARIADIC &&
        params.size() != param_count_) {
      throw Exception(ExceptionType::PARSER,
                      "expected " + std::to_string(param_count_) +
                          " arguments, got " + std::to_string(params.size()) +
                          ".");
    }
    params_ = params;
    is_set_ = true;
    if (immediate_parse_) {
      callback_(params_);
    }
  }
  void parse() {
    if (!immediate_parse_) {
      callback_(params_);
    }
  }

private:
  Flag(uint32_t param_count, std::string name, std::string description,
       flag_callback_t callback)
      : param_count_{param_count}, name_(name), description_{description},
        callback_{callback} {}

  template <class... ParamsT>
    requires(has_istream_operator<ParamsT> && ...)
  static void process_params(std::vector<std::string> &inputs,
                             ParamsT &...outputs) {
    uint32_t idx = 0;
    (
        [&] {
          std::istringstream arg_stream(inputs[idx++]);
          arg_stream >> outputs;
          if (!arg_stream ||
              arg_stream.peek() != std::char_traits<char>::eof()) {
            throw Exception(ExceptionType::PARSER,
                            "failed to parse argument " + inputs[idx - 1]);
          }
        }(),
        ...);
  }
  static void process_empty(std::vector<std::string> &) { /* do nothing */ }

private:
  bool is_set_ = false;
  bool immediate_parse_ = false;
  uint32_t param_count_;
  std::string name_;
  std::optional<char> alias_;
  std::string description_;
  flag_callback_t callback_;
  std::vector<std::string> params_;

  inline static std::unordered_set<char> alias_set_{};
};

} // namespace quikcli

#endif // QC_FLAG_H_
