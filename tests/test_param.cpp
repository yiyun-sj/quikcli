#include <optional>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <quikcli/param.hpp>

TEST_CASE("construct param using operator&") {
    auto p = quikcli::Flag<int>::required("count") & quikcli::Flag<int>::optional("max") &
             quikcli::Flag<int>::optional_with_default("min", 0) &
             quikcli::Flag<bool>::no_arg("verbose");
    static_assert(std::is_same_v<decltype(p), quikcli::Param<int, std::optional<int>, int, bool>>);
}

TEST_CASE("construct param using param wrapper helper function") {
    auto p_basecase = quikcli::param(quikcli::Flag<int>::required("count"));
    static_assert(std::is_same_v<decltype(p_basecase), quikcli::Param<int>>);
    auto p_multi =
        quikcli::param(quikcli::Flag<int>::required("count"), quikcli::Flag<int>::optional("max"),
                       quikcli::Flag<int>::optional_with_default("min", 0),
                       quikcli::Flag<bool>::no_arg("verbose"));
    static_assert(
        std::is_same_v<decltype(p_multi), quikcli::Param<int, std::optional<int>, int, bool>>);
}

TEST_CASE("add flag to param") {
    auto p = quikcli::Flag<int>::required("count") & quikcli::Flag<int>::optional("max") &
             quikcli::Flag<int>::optional_with_default("min", 0);
    auto p_added = p & quikcli::Flag<bool>::no_arg("verbose");
    static_assert(
        std::is_same_v<decltype(p_added), quikcli::Param<int, std::optional<int>, int, bool>>);
}

TEST_CASE("compose params") {
    auto p1 = quikcli::Flag<int>::required("count") & quikcli::Flag<int>::optional("max");
    auto p2 = quikcli::Flag<int>::optional_with_default("min", 0) &
              quikcli::Flag<bool>::no_arg("verbose");
    auto p_composed = p1 & p2;
    static_assert(
        std::is_same_v<decltype(p_composed), quikcli::Param<int, std::optional<int>, int, bool>>);
}

TEST_CASE("get specs from param") {
    auto p = quikcli::Flag<int>::required("count") & quikcli::Flag<int>::optional("max") &
             quikcli::Flag<int>::optional_with_default("min", 0) &
             quikcli::Flag<bool>::no_arg("verbose");
    auto s = p.specs();
    CHECK(s.size() == 4);
    CHECK(s[0]->long_name == "count");
    CHECK(s[1]->long_name == "max");
    CHECK(s[2]->long_name == "min");
    CHECK(s[3]->long_name == "verbose");
}

TEST_CASE("extract flag values from param") {
    auto p = quikcli::Flag<int>::required("count") & quikcli::Flag<int>::optional("max") &
             quikcli::Flag<int>::optional_with_default("min", 0) &
             quikcli::Flag<bool>::no_arg("verbose");
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
