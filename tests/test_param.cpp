#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <optional>
#include <quikcli/flag.hpp>
#include <quikcli/param.hpp>
#include <string>
#include <utility>

using namespace quikcli;

TEST_CASE("single flag as param") {
    Param<int> p = Flag<int>::required("count");
    static_assert(std::is_same_v<decltype(p), Param<int>>);
}

TEST_CASE("construct param using operator&") {
    auto p = Flag<int>::required("count") & Flag<int>::optional("max") &
             Flag<int>::optional_with_default("min", 0) & Flag<bool>::no_arg("verbose");
    static_assert(
        std::is_same_v<decltype(p), Param<std::tuple<int, std::optional<int>, int, bool>>>);
}

TEST_CASE("add flag to param") {
    auto p = Flag<int>::required("count") & Flag<int>::optional("max") &
             Flag<int>::optional_with_default("min", 0);
    auto p_added = p & Flag<bool>::no_arg("verbose");
    static_assert(
        std::is_same_v<decltype(p_added), Param<std::tuple<int, std::optional<int>, int, bool>>>);
}

TEST_CASE("add param to flag") {
    auto p = Flag<int>::optional("max") & Flag<int>::optional_with_default("min", 0) &
             Flag<bool>::no_arg("verbose");
    auto p_added = Flag<int>::required("count") & p;
    static_assert(
        std::is_same_v<decltype(p_added), Param<std::tuple<int, std::optional<int>, int, bool>>>);
}

TEST_CASE("compose params") {
    auto p1 = Flag<int>::required("count") & Flag<int>::optional("max");
    auto p2 = Flag<int>::optional_with_default("min", 0) & Flag<bool>::no_arg("verbose");
    auto p_composed = p1 & p2;
    static_assert(std::is_same_v<decltype(p_composed),
                                 Param<std::tuple<int, std::optional<int>, int, bool>>>);
}

TEST_CASE("get specs from param") {
    auto p = Flag<int>::required("count") & Flag<int>::optional("max") &
             Flag<int>::optional_with_default("min", 0) & Flag<bool>::no_arg("verbose");
    auto s = p.specs();
    CHECK(s.size() == 4);
    CHECK(s[0]->long_name == "count");
    CHECK(s[1]->long_name == "max");
    CHECK(s[2]->long_name == "min");
    CHECK(s[3]->long_name == "verbose");
}

TEST_CASE("extract flag values from param") {
    auto p = Flag<int>::required("count") & Flag<int>::optional("max") &
             Flag<int>::optional_with_default("min", 0) & Flag<bool>::no_arg("verbose");
    auto s = p.specs();
    CHECK(s.size() == 4);
    s[0]->raw_value = "1";
    s[1]->raw_value = "9";
    auto [cnt, max, min, verbose] = p.extract();
    CHECK(cnt == 1);
    CHECK(max == 9);
    CHECK(min == 0);
    CHECK(!verbose);
}

struct Server {
    std::string host;
    int port;
};

TEST_CASE("map param to custom type with operator|") {
    auto p = Flag<std::string>::required("host") & Flag<int>::required("port");
    auto server_param =
        p | [](std::string host, int port) { return Server{std::move(host), port}; };
    static_assert(std::is_same_v<decltype(server_param), Param<Server>>);

    auto s = server_param.specs();
    CHECK(s.size() == 2);
    s[0]->raw_value = "localhost";
    s[1]->raw_value = "8080";
    auto server = server_param.extract();
    CHECK(server.host == "localhost");
    CHECK(server.port == 8080);
}

struct Config {
    Server server;
    bool verbose;
};

TEST_CASE("compose mapped param with additional flag") {
    auto server_param = Flag<std::string>::required("host") & Flag<int>::required("port") |
                        [](std::string host, int port) { return Server{std::move(host), port}; };
    auto config_param = server_param & Flag<bool>::no_arg("verbose") |
                        [](Server s, bool v) { return Config{std::move(s), v}; };
    static_assert(std::is_same_v<decltype(config_param), Param<Config>>);

    auto s = config_param.specs();
    CHECK(s.size() == 3);
    s[0]->raw_value = "localhost";
    s[1]->raw_value = "8080";
    s[2]->raw_value = "";
    auto config = config_param.extract();
    CHECK(config.server.host == "localhost");
    CHECK(config.server.port == 8080);
    CHECK(config.verbose);
}

TEST_CASE("reuse param in multiple compositions") {
    auto host_port = Flag<std::string>::required("host") & Flag<int>::required("port");
    auto with_verbose = host_port & Flag<bool>::no_arg("verbose");
    auto with_debug = host_port & Flag<bool>::no_arg("debug");
    CHECK(with_verbose.specs().size() == 3);
    CHECK(with_debug.specs().size() == 3);
    CHECK(with_verbose.specs()[2]->long_name == "verbose");
    CHECK(with_debug.specs()[2]->long_name == "debug");
}
