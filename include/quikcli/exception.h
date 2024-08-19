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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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

#ifndef QC_EXCEPTION_H_
#define QC_EXCEPTION_H_

#include <cstdint>
#include <stdexcept>

namespace quikcli {

enum class ExceptionType : uint8_t {
  UNKNOWN = 0,
  CONFIGURATION = 1,
  PARSER = 2,
};

class Exception : public std::runtime_error {
public:
  Exception(ExceptionType type, const std::string& message)
    : std::runtime_error(ExceptionTypeToString(type) + " exception: " + message) {}

public:
  static std::string ExceptionTypeToString(ExceptionType type) {
    switch (type) {
      case ExceptionType::CONFIGURATION: {
        return "Configuration";
      };
      case ExceptionType::PARSER: {
        return "Parser";
      };
      default: {
        return "Unknown";
      }
    }
  } 
};

}

#endif // QC_EXCEPTION_H_
