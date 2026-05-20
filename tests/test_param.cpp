#include "quikcli/flag.hpp"

#include <optional>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <quikcli/param.hpp>

using namespace quikcli;

TEST_CASE("construct param using operator&") {
    auto p = Flag<int>::required("count") & Flag<int>::optional("max") &
             Flag<int>::optional_with_default("min", 0) & Flag<bool>::no_arg("verbose");
    static_assert(
        std::is_same_v<decltype(p),
                       Param<Flag<int>, Flag<int, std::optional<int>>, Flag<int>, Flag<bool>>>);
}

TEST_CASE("construct param using param wrapper helper function") {
    auto p_basecase = param(Flag<int>::required("count"));
    static_assert(std::is_same_v<decltype(p_basecase), Param<Flag<int>>>);
    auto p_multi = param(Flag<int>::required("count"), Flag<int>::optional("max"),
                         Flag<int>::optional_with_default("min", 0), Flag<bool>::no_arg("verbose"));
    static_assert(
        std::is_same_v<decltype(p_multi),
                       Param<Flag<int>, Flag<int, std::optional<int>>, Flag<int>, Flag<bool>>>);
}

TEST_CASE("add flag to param") {
    auto p = Flag<int>::required("count") & Flag<int>::optional("max") &
             Flag<int>::optional_with_default("min", 0);
    auto p_added = p & Flag<bool>::no_arg("verbose");
    static_assert(
        std::is_same_v<decltype(p_added),
                       Param<Flag<int>, Flag<int, std::optional<int>>, Flag<int>, Flag<bool>>>);
}

TEST_CASE("compose params") {
    auto p1 = Flag<int>::required("count") & Flag<int>::optional("max");
    auto p2 = Flag<int>::optional_with_default("min", 0) & Flag<bool>::no_arg("verbose");
    auto p_composed = p1 & p2;
    static_assert(
        std::is_same_v<decltype(p_composed),
                       Param<Flag<int>, Flag<int, std::optional<int>>, Flag<int>, Flag<bool>>>);
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
