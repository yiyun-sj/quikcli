#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstdlib>
#include <doctest/doctest.h>
#include <quikcli/flag.hpp>
#include <quikcli/formatter.hpp>
#include <quikcli/param.hpp>
#include <string>

TEST_CASE("format basic command no args") {

    auto p =
        quikcli::Flag<int>::required("count").alias('c').doc("number of things") &
        quikcli::Flag<bool>::no_arg("verbose").alias('v') &
        quikcli::Flag<float>::optional("max").doc("maximum value") &
        quikcli::Flag<float>::optional_with_default("min", 0.).doc("minimum value") &
        quikcli::Flag<std::string>::comma_delimited("variables").doc("extra states to process");

    std::vector<const quikcli::FlagSpec *> specs = p.specs();

    std::string help =
        quikcli::detail::Help::format_basic("./example", "An example program", {}, specs);

    CHECK(help == R"(An example program

  example

=== flags ===

   -c, --count=INT              . number of things
  [-v, --verbose]               . 
  [    --max=FLOAT]             . maximum value
  [    --min=FLOAT]             . minimum value(default: 0)
  [    --variables=STRING] ...  . extra states to process
  [-V, --version]               . print the version and exit
  [-h, --help]                  . print this help text and exit
)");
}

TEST_CASE("format basic command") {

    auto p =
        quikcli::Flag<int>::required("count").alias('c').doc("number of things") &
        quikcli::Flag<bool>::no_arg("verbose").alias('v') &
        quikcli::Flag<float>::optional("max").doc("maximum value") &
        quikcli::Flag<float>::optional_with_default("min", 0.).doc("minimum value") &
        quikcli::Flag<std::string>::comma_delimited("variables").doc("extra states to process") &
        quikcli::Flag<std::string>::anon("input") &
        quikcli::Flag<std::string>::anon_optional("output").doc(
            "file to send result to if provided") &
        quikcli::Flag<int>::anon_optional_with_default("compression", 70)
            .doc("compression factor [0, 100]") &
        quikcli::Flag<std::string>::anon_variadic("extras").doc("extra arguments");

    std::vector<const quikcli::FlagSpec *> specs = p.specs();

    std::string help =
        quikcli::detail::Help::format_basic("./example.exe", "An example program", {"sub"}, specs);

    CHECK(help == R"(An example program

  example.exe sub <input> [output] [compression] [extras ...]

=== arguments ===

  input=STRING     . 
  output=STRING    . file to send result to if provided
  compression=INT  . compression factor [0, 100](default: 70)
  extras=STRING    . extra arguments

=== flags ===

   -c, --count=INT              . number of things
  [-v, --verbose]               . 
  [    --max=FLOAT]             . maximum value
  [    --min=FLOAT]             . minimum value(default: 0)
  [    --variables=STRING] ...  . extra states to process
  [-V, --version]               . print the version and exit
  [-h, --help]                  . print this help text and exit
)");
}

TEST_CASE("format group command") {
    std::vector<std::pair<std::string, std::string>> s{{"example", "An example program"}};
    std::string help = quikcli::detail::Help::format_group(
        "/path/to/example", "An example program with subcommands", {}, s);

    CHECK(help == R"(An example program with subcommands

  example <subcommand>

=== subcommands ===

  example  . An example program
  version  . print version information
  help     . explain a given subcommand
)");
}
