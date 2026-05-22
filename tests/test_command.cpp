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

struct AddResult {
    std::string path;
    std::vector<std::string> tags;
    bool verbose;
};

struct SearchResult {
    std::string query;
    int limit;
};

struct CopyResult {
    std::string dest;
    std::vector<std::string> sources;
};

TEST_CASE("end-to-end: archive CLI") {
    using namespace quikcli;

    std::optional<AddResult> add_result;
    std::optional<SearchResult> search_result;
    std::optional<CopyResult> copy_result;

    auto add_cmd = Command::basic(
        "Add a file to the archive with tags.",
        Flag<std::string>::anon("path").doc("file to archive") &
                Flag<std::string>::comma_delimited("tags").doc("comma-separated tags") &
                Flag<bool>::no_arg("verbose").alias('v').doc("verbose output") |
            [](std::string path, std::vector<std::string> tags, bool verbose) -> AddResult {
            return {std::move(path), std::move(tags), verbose};
        },
        [&](AddResult r) { add_result = std::move(r); });

    auto search_cmd = Command::basic(
        "Search the archive by query string.",
        Flag<std::string>::anon("query").doc("search query") &
                Flag<int>::optional_with_default("limit", 10).alias('n').doc("max results") |
            [](std::string query, int limit) -> SearchResult { return {std::move(query), limit}; },
        [&](SearchResult r) { search_result = std::move(r); });

    auto copy_cmd = Command::basic(
        "Copy files to a destination directory.",
        Flag<std::string>::required("dest").alias('d').doc("destination directory") &
                Flag<std::string>::anon_variadic("sources").doc("files to copy") |
            [](std::string dest, std::vector<std::string> sources) -> CopyResult {
            return {std::move(dest), std::move(sources)};
        },
        [&](CopyResult r) { copy_result = std::move(r); });

    auto cli = Command::group("Archive tool: manage a local file archive.",
                              {{"add", std::move(add_cmd)},
                               {"search", std::move(search_cmd)},
                               {"copy", std::move(copy_cmd)}});

    SUBCASE("add: all flags parsed and piped into struct") {
        Argv arg{{"prog", "add", "notes.txt", "--tags", "work,todo,urgent", "-v"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        REQUIRE(add_result.has_value());
        CHECK(add_result->path == "notes.txt");
        CHECK(add_result->tags == std::vector<std::string>{"work", "todo", "urgent"});
        CHECK(add_result->verbose);
        CHECK(out.str().empty());
        CHECK(err.str().empty());
    }

    SUBCASE("add: --tags=value inline form; verbose absent defaults to false") {
        Argv arg{{"prog", "add", "readme.md", "--tags=docs,api"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        REQUIRE(add_result.has_value());
        CHECK(add_result->path == "readme.md");
        CHECK(add_result->tags == std::vector<std::string>{"docs", "api"});
        CHECK(!add_result->verbose);
    }

    SUBCASE("add: missing required positional writes to err, callback not invoked") {
        Argv arg{{"prog", "add", "--tags", "work"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        CHECK(!add_result.has_value());
        CHECK(out.str().empty());
        CHECK(!err.str().empty());
    }

    SUBCASE("search: default limit used when flag absent") {
        Argv arg{{"prog", "search", "hello"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        REQUIRE(search_result.has_value());
        CHECK(search_result->query == "hello");
        CHECK(search_result->limit == 10);
    }

    SUBCASE("search: explicit -n overrides default") {
        Argv arg{{"prog", "search", "hello", "-n", "3"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        REQUIRE(search_result.has_value());
        CHECK(search_result->query == "hello");
        CHECK(search_result->limit == 3);
    }

    SUBCASE("copy: variadic sources collected in order") {
        Argv arg{{"prog", "copy", "--dest", "/tmp/bak", "a.txt", "b.txt", "c.txt"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        REQUIRE(copy_result.has_value());
        CHECK(copy_result->dest == "/tmp/bak");
        CHECK(copy_result->sources == std::vector<std::string>{"a.txt", "b.txt", "c.txt"});
    }

    SUBCASE("copy: zero sources is accepted") {
        Argv arg{{"prog", "copy", "-d", "/tmp/bak"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        REQUIRE(copy_result.has_value());
        CHECK(copy_result->dest == "/tmp/bak");
        CHECK(copy_result->sources.empty());
    }

    SUBCASE("group help lists all subcommands") {
        Argv arg{{"prog", "help"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        CHECK(out.str().find("add") != std::string::npos);
        CHECK(out.str().find("search") != std::string::npos);
        CHECK(out.str().find("copy") != std::string::npos);
        CHECK(err.str().empty());
    }

    SUBCASE("group version") {
        Argv arg{{"prog", "version"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        CHECK(out.str() == "1.0.0\n");
        CHECK(err.str().empty());
    }

    SUBCASE("nested add --help goes to out, callback not invoked") {
        Argv arg{{"prog", "add", "--help"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        CHECK(!out.str().empty());
        CHECK(out.str().find("tags") != std::string::npos);
        CHECK(out.str().find("verbose") != std::string::npos);
        CHECK(err.str().empty());
        CHECK(!add_result.has_value());
    }

    SUBCASE("nested search --version goes to out") {
        Argv arg{{"prog", "search", "--version"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        CHECK(out.str() == "1.0.0\n");
        CHECK(err.str().empty());
    }

    SUBCASE("unknown subcommand error goes to err") {
        Argv arg{{"prog", "delete", "notes.txt"}};
        std::ostringstream out, err;
        cli.run(arg.argc(), arg.argv(), "1.0.0", out, err);

        CHECK(out.str().empty());
        CHECK(err.str().find("delete") != std::string::npos);
    }
}
