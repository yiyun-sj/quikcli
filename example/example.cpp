#include "quikcli/component.h"
#include "quikcli/quikcli.h"
#include <chrono>
#include <memory>
#include <thread>

std::unique_ptr<quikcli::Component> make_welcome_message() {
  std::vector<std::string> outputs;
  outputs.emplace_back("  ____        _ _     _____ _ _ ");
  outputs.emplace_back(" / __ \\      (_) |   / ____| (_)");
  outputs.emplace_back("| |  | |_   _ _| | _| |    | |_ ");
  outputs.emplace_back("| |  | | | | | | |/ / |    | | |");
  outputs.emplace_back("| |__| | |_| | |   <| |____| | |");
  outputs.emplace_back(" \\___\\_\\\\__,_|_|_|\\_\\\\_____|_|_|");
  return std::make_unique<quikcli::Display>(outputs);
}

std::unique_ptr<quikcli::Component> make_init_loader() {
  return std::make_unique<quikcli::Loader>([](quikcli::Loader &loader) {
    std::thread loading([&] {
      for (int i = 1; i <= 100; i++) {
        loader.update(0.01 * i);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
      }
    });
    loading.detach();
  });
}

int main(int argc, char *argv[]) {
  quikcli::QuikCli cli{"quikcli", "0.0.1"};
  cli.parse_flags(argc, argv);
  cli.push_component(make_welcome_message());
  cli.push_component(make_init_loader());
  cli.run();
}
