#include "quikcli/quikcli.h"

int main(int argc, char *argv[]) {
  quikcli::QuikCli cli {"quikcli", "0.0.1"};
  cli.parse_flags(argc, argv);
  cli.run();
}
