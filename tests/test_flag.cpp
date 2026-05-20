#include "quikcli/fwd.hpp"

#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <quikcli/flag.hpp>

TEST_CASE("required flag") {
    auto f = quikcli::Flag<int>::required("name").doc("doc").alias('n');
    CHECK(f.spec().kind == quikcli::FlagKind::Required);
    CHECK(f.spec().long_name == "name");
    CHECK(f.spec().doc_str == "doc");
    CHECK(f.spec().short_alias == 'n');
    CHECK(!f.default_value().has_value());
}

TEST_CASE("optional flag") {
    auto f = quikcli::Flag<std::string>::optional("name");
    CHECK(f.spec().kind == quikcli::FlagKind::Optional);
    CHECK(f.spec().long_name == "name");
    CHECK(!f.default_value().has_value());
}

TEST_CASE("optional flag with default") {
    auto f = quikcli::Flag<std::string>::optional_with_default("name", "default");
    CHECK(f.spec().kind == quikcli::FlagKind::OptionalWithDefault);
    CHECK(f.spec().long_name == "name");
    CHECK(f.default_value() == "default");
}

TEST_CASE("no_arg flag") {
    auto f = quikcli::Flag<bool>::no_arg("name");
    CHECK(f.spec().kind == quikcli::FlagKind::NoArg);
    CHECK(f.spec().long_name == "name");
}

TEST_CASE("anon flag") {
    auto f = quikcli::Flag<int>::anon();
    CHECK(f.spec().kind == quikcli::FlagKind::Anon);
    CHECK(f.spec().long_name == "");
}

TEST_CASE("anon_optional flag") {
    auto f = quikcli::Flag<int>::anon_optional();
    CHECK(f.spec().kind == quikcli::FlagKind::AnonOptional);
    CHECK(f.spec().long_name == "");
}

TEST_CASE("anon_optional_with_default flag") {
    auto f = quikcli::Flag<int>::anon_optional_with_default(1);
    CHECK(f.spec().kind == quikcli::FlagKind::AnonOptionalWithDefault);
    CHECK(f.spec().long_name == "");
    CHECK(f.default_value() == 1);
}

TEST_CASE("anon_variadic flag") {
    auto f = quikcli::Flag<int>::anon_variadic();
    CHECK(f.spec().kind == quikcli::FlagKind::AnonVariadic);
    CHECK(f.spec().long_name == "");
    CHECK(f.spec().raw_values.empty());
}

TEST_CASE("comma_delimited flag") {
    auto f = quikcli::Flag<int>::comma_delimited("name");
    CHECK(f.spec().kind == quikcli::FlagKind::CommaDelimited);
    CHECK(f.spec().long_name == "name");
}

TEST_CASE("empty flag initial values") {
    auto f = quikcli::Flag<int>::required("name");
    CHECK(f.spec().kind == quikcli::FlagKind::Required);
    CHECK(f.spec().long_name == "name");
    CHECK(f.spec().doc_str == "");
    CHECK(!f.spec().short_alias.has_value());
    CHECK(!f.spec().raw_value.has_value());
    CHECK(f.spec().raw_values.empty());
}

TEST_CASE("invalid names") {
    CHECK_THROWS_AS(quikcli::detail::validate_flag_name(""), quikcli::FlagNameError);
    CHECK_THROWS_AS(quikcli::detail::validate_flag_name("-"), quikcli::FlagNameError);
    CHECK_THROWS_AS(quikcli::detail::validate_flag_name("--name"), quikcli::FlagNameError);
    CHECK_THROWS_AS(quikcli::Flag<int>::required("-name"), quikcli::FlagNameError);
}

TEST_CASE("extract parsed value from required flag") {
    auto f = quikcli::Flag<int>::required("count");
    CHECK_THROWS_AS(f.extract(), quikcli::ParseError);
    f.spec().raw_value = "1";
    CHECK(f.extract() == 1);
}

TEST_CASE("extract parsed value from noarg flag") {
    auto f = quikcli::Flag<bool>::no_arg("verbose");
    CHECK(!f.extract());
    f.spec().raw_value = "";
    CHECK(f.extract());
}

TEST_CASE("extract parsed value from optional flag") {
    auto f = quikcli::Flag<int>::optional("count");
    CHECK(!f.extract().has_value());
    f.spec().raw_value = "1";
    CHECK(f.extract() == 1);
}

TEST_CASE("extract parsed value from optional_with_default flag") {
    auto f = quikcli::Flag<int>::optional_with_default("count", 0);
    CHECK(f.extract() == 0);
    f.spec().raw_value = "1";
    CHECK(f.extract() == 1);
}

TEST_CASE("extract parsed value from comma_delimited flag") {
    auto f = quikcli::Flag<int>::comma_delimited("counts");
    CHECK(f.extract() == std::vector<int>{});
    f.spec().raw_value = "1";
    CHECK(f.extract() == std::vector<int>{1});
    f.spec().raw_value = "1,2,3,4";
    CHECK(f.extract() == std::vector<int>{1, 2, 3, 4});
}

TEST_CASE("extract parsed value from anon flag") {
    auto f = quikcli::Flag<int>::anon();
    CHECK_THROWS_AS(f.extract(), quikcli::ParseError);
    f.spec().raw_value = "1";
    CHECK(f.extract() == 1);
}

TEST_CASE("extract parsed value from anon_optional flag") {
    auto f = quikcli::Flag<int>::anon_optional();
    CHECK(!f.extract().has_value());
    f.spec().raw_value = "1";
    CHECK(f.extract() == 1);
}

TEST_CASE("extract parsed value from anon_optional_with_default flag") {
    auto f = quikcli::Flag<int>::anon_optional_with_default(0);
    CHECK(f.extract() == 0);
    f.spec().raw_value = "1";
    CHECK(f.extract() == 1);
}

TEST_CASE("extract parsed value from anon_variadic flag") {
    auto f = quikcli::Flag<int>::anon_variadic();
    CHECK(f.extract() == std::vector<int>{});
    f.spec().raw_values.push_back("1");
    CHECK(f.extract() == std::vector<int>{1});
    f.spec().raw_values.push_back("2");
    f.spec().raw_values.push_back("3");
    f.spec().raw_values.push_back("4");
    CHECK(f.extract() == std::vector<int>{1, 2, 3, 4});
}