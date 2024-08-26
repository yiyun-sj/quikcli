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

#ifndef QC_WRITER_H_
#define QC_WRITER_H_

#include <iostream>
#include <string>
#include <vector>

#include <sys/ioctl.h>
#include <unistd.h>

namespace quikcli {

class Writer {
public:
  Writer() { init(); }

  Writer(Writer &) = delete;
  Writer &operator=(Writer &) = delete;
  Writer(Writer &&) = default;
  Writer &operator=(Writer &&) = default;

public:
  /* Getters & Setters */
  int width() { return width_; }
  Writer &reset_cursor() {
    reset_cursor_ = true;
    return *this;
  }

  /* Runtime */
  void newline() { std::cout << std::endl; }
  void out(std::string &output) {
    std::cout << output << std::endl;
    if (reset_cursor_) {
      std::cout << "\033[1A";
    }
    std::cout.flush();
    reset();
  }
  void out(std::vector<std::string> &outputs) {
    for (std::string &output : outputs) {
      std::cout << output << std::endl;
    }
    if (reset_cursor_) {
      std::cout << "\033[" + std::to_string(outputs.size()) + "A";
    }
    std::cout.flush();
    reset();
  }

private:
  void init() {
    winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    width_ = w.ws_col;
  }
  void reset() { reset_cursor_ = false; }

private:
  unsigned short width_;
  bool reset_cursor_ = false;
};

} // namespace quikcli

#endif // QC_WRITER_H_
