#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <quikcli/command.hpp>
#include <sstream>
#include <string>
#include <vector>

struct Argv {
    std::vector<std::string> storage;
    std::vector<char *> ptrs;

    explicit Argv(std::vector<std::string> args) : storage(std::move(args)) {
        ptrs.reserve(storage.size());
        for (auto &s : storage)
            ptrs.push_back(s.data());
    }

    int argc() const { return (int)ptrs.size(); }
    char **argv() { return ptrs.data(); }
};

TEST_CASE("basic: parse error for unknown flag writes to err, not out") {
    auto cmd =
        quikcli::Command::basic("test command", quikcli::Flag<int>::required("count"), [](int) {});

    Argv arg{{"prog", "--no-such-flag"}};
    std::ostringstream out, err;
    cmd.run(arg.argc(), arg.argv(), "1.0", out, err);

    CHECK(out.str().empty());
    CHECK(err.str() == R"(Error parsing command line:

  unknown flag --no-such-flag

For usage information, run

  prog --help

)");
}

TEST_CASE("basic: --help prints to out") {
    auto cmd =
        quikcli::Command::basic("a summary", quikcli::Flag<int>::required("count"), [](int) {});

    Argv arg{{"prog", "--help"}};
    std::ostringstream out, err;
    cmd.run(arg.argc(), arg.argv(), "1.0", out, err);

    CHECK(out.str() == R"(a summary

  prog

=== flags ===

       --count=INT   . 
  [-V, --version]    . print the version and exit
  [-h, --help]       . print this help text and exit
)");
    CHECK(err.str().empty());
    CHECK(out.str().find("a summary") != std::string::npos);
    CHECK(out.str().find("prog") != std::string::npos);
}

TEST_CASE("basic: -h prints to out") {
    auto cmd =
        quikcli::Command::basic("a summary", quikcli::Flag<int>::required("count"), [](int) {});

    Argv arg{{"prog", "-h"}};
    std::ostringstream out, err;
    cmd.run(arg.argc(), arg.argv(), "1.0", out, err);

    CHECK(out.str() == R"(a summary

  prog

=== flags ===

       --count=INT   . 
  [-V, --version]    . print the version and exit
  [-h, --help]       . print this help text and exit
)");
    CHECK(err.str().empty());
}

TEST_CASE("basic: --version prints version string to out") {
    auto cmd = quikcli::Command::basic("test", quikcli::Flag<int>::required("count"), [](int) {});

    Argv arg{{"prog", "--version"}};
    std::ostringstream out, err;
    cmd.run(arg.argc(), arg.argv(), "2.3.4", out, err);

    CHECK(out.str() == "2.3.4\n");
    CHECK(err.str().empty());
}

TEST_CASE("group: 'help' prints group summary and subcommand list to out") {
    auto sub =
        quikcli::Command::basic("sub summary", quikcli::Flag<int>::required("count"), [](int) {});
    auto cmd = quikcli::Command::group("root summary", {{"sub", std::move(sub)}});

    Argv arg{{"prog", "help"}};
    std::ostringstream out, err;
    cmd.run(arg.argc(), arg.argv(), "1.0", out, err);

    CHECK(out.str() == R"(root summary

  prog <subcommand>

=== subcommands ===

  sub      . sub summary
  version  . print version information
  help     . explain a given subcommand
)");
    CHECK(err.str().empty());
    CHECK(out.str().find("root summary") != std::string::npos);
    CHECK(out.str().find("sub") != std::string::npos);
}

TEST_CASE("group: 'version' prints version string to out") {
    auto cmd = quikcli::Command::group("root summary", {});

    Argv arg{{"prog", "version"}};
    std::ostringstream out, err;
    cmd.run(arg.argc(), arg.argv(), "4.5.6", out, err);

    CHECK(out.str() == "4.5.6\n");
    CHECK(err.str().empty());
}

TEST_CASE("group: missing subcommand writes error to err") {
    auto cmd = quikcli::Command::group("root summary", {});

    Argv arg{{"prog"}};
    std::ostringstream out, err;
    cmd.run(arg.argc(), arg.argv(), "1.0", out, err);

    CHECK(out.str().empty());
    CHECK(err.str() == R"(Error parsing command line:

  missing subcommand

For usage information, run

  prog help

)");
}

TEST_CASE("group: unknown subcommand writes error to err") {
    auto cmd = quikcli::Command::group("root summary", {});

    Argv arg{{"prog", "unknown"}};
    std::ostringstream out, err;
    cmd.run(arg.argc(), arg.argv(), "1.0", out, err);

    CHECK(out.str().empty());
    CHECK(err.str() == R"(Error parsing command line:

  unknown subcommand unknown

For usage information, run

  prog help

)");
}
