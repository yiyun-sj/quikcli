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

TEST_CASE("empty flag initial values") {
    auto f = quikcli::Flag<int>::required("name");
    CHECK(f.spec().kind == quikcli::FlagKind::Required);
    CHECK(f.spec().long_name == "name");
    CHECK(f.spec().doc_str == "");
    CHECK(!f.spec().short_alias.has_value());
    CHECK(!f.spec().raw_value.has_value());
}

TEST_CASE("invalid names") {
    CHECK_THROWS_AS(quikcli::detail::validate_flag_name(""), quikcli::FlagNameError);
    CHECK_THROWS_AS(quikcli::detail::validate_flag_name("-"), quikcli::FlagNameError);
    CHECK_THROWS_AS(quikcli::detail::validate_flag_name("--name"), quikcli::FlagNameError);
    CHECK_THROWS_AS(quikcli::Flag<int>::required("-name"), quikcli::FlagNameError);
}
