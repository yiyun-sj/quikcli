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

#ifndef QC_CONSTANTS_H_
#define QC_CONSTANTS_H_

#include <cstdint>
namespace quikcli {

struct DefaultFlagNames {
  static constexpr char version[] = "version";
  static constexpr char help[] = "help";
};

struct DefaultFlagAliases {
  static constexpr char version = 'V';
  static constexpr char help = 'h';
};

struct DefaultFlagDescription {
  static constexpr char version[] = "print version information then exit.";
  static constexpr char help[] = "print help message then exit.";
};

struct FlagParamSize {
  static constexpr uint32_t EMPTY = 0;
  static constexpr uint32_t VARIADIC = 0xFFFFFFFF;
};

} // namespace quikcli

#endif // QC_CONSTANTS_H_
