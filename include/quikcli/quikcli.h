#ifndef QC_QUIKCLI_H_
#define QC_QUIKCLI_H_

#include <string>
#include <iostream>
#include <unordered_map>
#include <functional>

#include "constants.h"

namespace quikcli {

class QuikCli {
public:
  /* Constructors & Destructors - No Copy Default Move */
  QuikCli()
    : QuikCli{DefaultParams::name, DefaultParams::version} {}
  QuikCli(std::string name, std::string version)
    : isActive{true}, name{name}, version{version} {
    // addArg(ConfigNames::help);
    addArg(ArgNames::version, versionArgFunc);
  }

  QuikCli(QuikCli&) = delete;
  QuikCli& operator = (QuikCli&) = delete;
  QuikCli(QuikCli&&) = default;
  QuikCli& operator = (QuikCli&&) = default;

  ~QuikCli() = default;

public:
  /* Getters & Setters */
  std::string getName() const { return name; }
  void setName(std::string name_) { name = name_; }
  std::string getVersion() const { return version; }
  void setVersion(std::string version_) { version = version_; }

  /* Configurations */
  void addArg(std::string arg, std::function<void(QuikCli&)> func) {
    args.insert_or_assign(arg, func);
  }

  /* Run-Time Functions */
  void processArgs(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
      if (!isActive) {
        return;
      }
      if (args.contains(argv[i])) {
        args[argv[i]](*this);
      } else {
        std::cout << "Invalid argument '" << argv[i] << "'\n";
        exit();
      }
    }
  }
  void run() {
    if (!isActive) {
      return;
    }
    std::string line;
    std::cout << name << "> ";
    while (std::cin >> line) {
      std::cout << line << '\n';
      if (!isActive) {
        return;
      }
      std::cout << name << "> ";
    }
  }
  void exit() {
    isActive = false;
  }

private:
  /* Default Argument Functions */
  static void versionArgFunc(QuikCli& cli) {
    std::cout << cli.getName() << " version " << cli.getVersion() << '\n';
    cli.exit();
  }

private:
  bool isActive;
  std::string name;
  std::string version;
  std::unordered_map<std::string, std::function<void(QuikCli&)>> args;
};

}

#endif // QC_QUIKCLI_H_
