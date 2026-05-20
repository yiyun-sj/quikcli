#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <quikcli/parser.hpp>

using quikcli::detail::Parser;

static std::vector<char *> make_argv(std::vector<std::string> &args) {
    std::vector<char *> argv;
    argv.reserve(args.size() + 1);
    for (auto &s : args)
        argv.push_back(s.data());
    argv.push_back(nullptr);
    return argv;
}

TEST_CASE("--flag value sets raw_value") {
    auto count = quikcli::Flag<int>::required("count");
    Parser parser({&count.spec()});

    auto args = std::vector<std::string>{"prog", "--count", "42"};
    auto argv = make_argv(args);
    parser.parse((int)args.size(), argv.data());

    CHECK(count.spec().raw_value == "42");
}

TEST_CASE("--flag=value sets raw_value") {
    auto name = quikcli::Flag<std::string>::optional_with_default("name", std::string("world"));
    Parser parser({&name.spec()});

    auto args = std::vector<std::string>{"prog", "--name=Alice"};
    auto argv = make_argv(args);
    parser.parse((int)args.size(), argv.data());

    CHECK(name.spec().raw_value == "Alice");
}

TEST_CASE("-v sets no-arg flag") {
    auto verbose = quikcli::Flag<bool>::no_arg("verbose").alias('v');
    Parser parser({&verbose.spec()});

    auto args = std::vector<std::string>{"prog", "-v"};
    auto argv = make_argv(args);
    parser.parse((int)args.size(), argv.data());

    CHECK(verbose.spec().raw_value.has_value());
}

TEST_CASE("-abc bundles three no-arg flags") {
    auto fa = quikcli::Flag<bool>::no_arg("aaa").alias('a');
    auto fb = quikcli::Flag<bool>::no_arg("bbb").alias('b');
    auto fc = quikcli::Flag<bool>::no_arg("ccc").alias('c');
    Parser parser({&fa.spec(), &fb.spec(), &fc.spec()});

    auto args = std::vector<std::string>{"prog", "-abc"};
    auto argv = make_argv(args);
    parser.parse((int)args.size(), argv.data());

    CHECK(fa.spec().raw_value.has_value());
    CHECK(fb.spec().raw_value.has_value());
    CHECK(fc.spec().raw_value.has_value());
}

TEST_CASE("-n5 short flag with inline value") {
    auto count = quikcli::Flag<int>::required("count").alias('n');
    Parser parser({&count.spec()});

    auto args = std::vector<std::string>{"prog", "-n5"};
    auto argv = make_argv(args);
    parser.parse((int)args.size(), argv.data());

    CHECK(count.spec().raw_value == "5");
}

TEST_CASE("-n space value short flag") {
    auto count = quikcli::Flag<int>::required("count").alias('n');
    Parser parser({&count.spec()});

    auto args = std::vector<std::string>{"prog", "-n", "99"};
    auto argv = make_argv(args);
    parser.parse((int)args.size(), argv.data());

    CHECK(count.spec().raw_value == "99");
}

TEST_CASE("unknown long flag throws") {
    Parser parser({});

    auto args = std::vector<std::string>{"prog", "--foo"};
    auto argv = make_argv(args);
    CHECK_THROWS_AS(parser.parse((int)args.size(), argv.data()), quikcli::ParseError);
}

TEST_CASE("unknown short flag throws") {
    Parser parser({});

    auto args = std::vector<std::string>{"prog", "-z"};
    auto argv = make_argv(args);
    CHECK_THROWS_AS(parser.parse((int)args.size(), argv.data()), quikcli::ParseError);
}

TEST_CASE("duplicate long flag throws") {
    auto count = quikcli::Flag<int>::required("count");
    Parser parser({&count.spec()});

    auto args = std::vector<std::string>{"prog", "--count", "1", "--count", "2"};
    auto argv = make_argv(args);
    CHECK_THROWS_AS(parser.parse((int)args.size(), argv.data()), quikcli::ParseError);
}

// TODO: decide if this is actually okay
TEST_CASE("duplicate no-arg flag throws") {
    auto verbose = quikcli::Flag<bool>::no_arg("verbose").alias('v');
    Parser parser({&verbose.spec()});

    auto args = std::vector<std::string>{"prog", "-v", "--verbose"};
    auto argv = make_argv(args);
    CHECK_THROWS_AS(parser.parse((int)args.size(), argv.data()), quikcli::ParseError);
}

TEST_CASE("--help sets help_requested") {
    Parser parser({});

    auto args = std::vector<std::string>{"prog", "--help"};
    auto argv = make_argv(args);
    auto result = parser.parse((int)args.size(), argv.data());

    CHECK(result.help_requested);
    CHECK(!result.version_requested);
}

TEST_CASE("-h sets help_requested") {
    Parser parser({});

    auto args = std::vector<std::string>{"prog", "-h"};
    auto argv = make_argv(args);
    auto result = parser.parse((int)args.size(), argv.data());

    CHECK(result.help_requested);
}

TEST_CASE("--help skips parser throw") {
    Parser parser({});

    auto args = std::vector<std::string>{"prog", "--unknown", "--help", "--unknown"};
    auto argv = make_argv(args);
    auto result = parser.parse((int)args.size(), argv.data());

    CHECK(result.help_requested);
}

TEST_CASE("--version sets version_requested") {
    Parser parser({});

    auto args = std::vector<std::string>{"prog", "--version"};
    auto argv = make_argv(args);
    auto result = parser.parse((int)args.size(), argv.data());

    CHECK(result.version_requested);
    CHECK(!result.help_requested);
}

TEST_CASE("-V sets version_requested") {
    Parser parser({});

    auto args = std::vector<std::string>{"prog", "-V"};
    auto argv = make_argv(args);
    auto result = parser.parse((int)args.size(), argv.data());

    CHECK(result.version_requested);
}

TEST_CASE("anon positional consumed from argv") {
    auto arg = quikcli::Flag<std::string>::anon("file");
    Parser parser({&arg.spec()});

    auto args = std::vector<std::string>{"prog", "hello.txt"};
    auto argv = make_argv(args);
    parser.parse((int)args.size(), argv.data());

    CHECK(arg.spec().raw_value == "hello.txt");
}

TEST_CASE("-- stops flag parsing") {
    auto verbose = quikcli::Flag<bool>::no_arg("verbose");
    auto positionals = quikcli::Flag<std::string>::anon_variadic("positionals");
    Parser parser({&verbose.spec(), &positionals.spec()});

    auto args = std::vector<std::string>{"prog", "--", "--verbose", "extra"};
    auto argv = make_argv(args);
    parser.parse((int)args.size(), argv.data());

    CHECK(!verbose.spec().raw_value.has_value());
    CHECK(positionals.spec().raw_values == std::vector<std::string>{"--verbose", "extra"});
}

TEST_CASE("anon variadic collects all positionals in order") {
    auto positionals = quikcli::Flag<std::string>::anon_variadic("positionals");
    Parser parser({&positionals.spec()});

    auto args = std::vector<std::string>{"prog", "foo", "bar", "baz"};
    auto argv = make_argv(args);
    parser.parse((int)args.size(), argv.data());

    CHECK(positionals.spec().raw_values == std::vector<std::string>{"foo", "bar", "baz"});
}

TEST_CASE("too many positionals") {
    Parser parser({});

    auto args = std::vector<std::string>{"prog", "foo", "bar", "baz"};
    auto argv = make_argv(args);

    CHECK_THROWS_AS(parser.parse((int)args.size(), argv.data()), quikcli::ParseError);
}

TEST_CASE("flags and positionals coexist") {
    auto count = quikcli::Flag<int>::required("count").alias('n');
    auto verbose = quikcli::Flag<bool>::no_arg("verbose").alias('v');
    auto file = quikcli::Flag<std::string>::anon("file");
    auto rest = quikcli::Flag<std::string>::anon_variadic("positionals");
    Parser parser({&count.spec(), &verbose.spec(), &file.spec(), &rest.spec()});

    auto args = std::vector<std::string>{"prog", "-n", "7", "pos1", "-v", "pos2", "--", "pos3"};
    auto argv = make_argv(args);
    parser.parse((int)args.size(), argv.data());

    CHECK(count.spec().raw_value == "7");
    CHECK(verbose.spec().raw_value.has_value());
    CHECK(file.spec().raw_value == "pos1");
    CHECK(rest.spec().raw_values == std::vector<std::string>{"pos2", "pos3"});
}
