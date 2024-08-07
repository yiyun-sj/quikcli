#ifndef QC_QUIKCLI_H
#define QC_QUIKCLI_H

#include <string>
#include <iostream>

namespace quikcli {

class QuikCli {

public:
  QuikCli() = default;
  // remove copy ctors
  QuikCli(QuikCli&) = delete;
  QuikCli operator = (QuikCli&) = delete;
  // allow move ctors
  QuikCli(QuikCli&&) = delete;
  QuikCli operator = (QuikCli&&) = delete;

  ~QuikCli() = default;

public:
  void addConfig();
  void run() {
    std::string line;
    while (std::cin >> line) {
      std::cout << line << '\n';
    }
  }
};

}

#endif
