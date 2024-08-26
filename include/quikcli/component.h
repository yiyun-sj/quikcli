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

#ifndef QC_COMPONENT_H_
#define QC_COMPONENT_H_

#include <functional>
#include <string>
#include <thread>

#include "quikcli/writer.h"

namespace quikcli {

class Loader;

using empty_callback_t = std::function<void()>;
using str_callback_t = std::function<void(std::string &)>;
using str_vec_callback_t = std::function<void(std::vector<std::string> &)>;

using loader_trigger_t = std::function<void(Loader &)>;

class Component {
public:
  Component() {}
  ~Component() = default;

  Component(Component &) = default;
  Component &operator=(Component &) = default;
  Component(Component &&) = default;
  Component &operator=(Component &&) = default;

  virtual void run(Writer &writer) = 0;
};

class Display : public Component {
public:
  Display() : Display({}, [] {}) {}
  Display(std::vector<std::string> outputs)
      : Display(std::move(outputs), [] {}) {}
  Display(std::vector<std::string> outputs, empty_callback_t callback)
      : outputs_{std::move(outputs)}, callback_{std::move(callback)} {}

  Display(Display &) = default;
  Display &operator=(Display &) = default;
  Display(Display &&) = default;
  Display &operator=(Display &&) = default;

public:
  void run(Writer &writer) override {
    writer.out(outputs_);
    callback_();
  };

private:
  std::vector<std::string> outputs_;
  empty_callback_t callback_;
};

class Loader : public Component {
public:
  Loader(loader_trigger_t trigger) : Loader(std::move(trigger), [] {}) {}
  Loader(loader_trigger_t trigger, empty_callback_t callback)
      : trigger_{std::move(trigger)}, callback_{std::move(callback)} {}

  Loader(Loader &) = default;
  Loader &operator=(Loader &) = default;
  Loader(Loader &&) = default;
  Loader &operator=(Loader &&) = default;

public:
  // TODO: Add check on terminal width & find better way to signal update
  void run(Writer &writer) override {
    trigger_(*this);
    unsigned short width = writer.width();
    std::string output;
    output.resize(width, ' ');
    output[0] = '[';
    output[width - 6] = ']';
    output[width - 1] = '%';
    while (progress_ < 1.0) {
      if (update_) {
        int bar_progress = (width - 6) * progress_;
        int int_progress = 100 * progress_;
        output.replace(1, bar_progress, bar_progress, '=');
        output[bar_progress + 1] = '>';
        output.replace(width - 4, 2, std::to_string(int_progress));
        writer.reset_cursor();
        writer.out(output);
        update_ = false;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    writer.newline();
    callback_();
  };

  void update(double progress) {
    progress_ = progress;
    update_ = true;
  }

private:
  loader_trigger_t trigger_;
  empty_callback_t callback_;
  double progress_ = 0;
  bool update_ = false;
};

} // namespace quikcli

#endif // QC_COMPONENT_H_
