#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <quikcli/arg_type.hpp>

TEST_CASE("BOOL ArgType") {
    CHECK(quikcli::ArgType<bool>::parse("true") == true);
    CHECK(quikcli::ArgType<bool>::parse("false") == false);
    CHECK_THROWS_AS(quikcli::ArgType<bool>::parse("anything else"), quikcli::ParseError);
}

TEST_CASE("STRING ArgType") {
    CHECK(quikcli::ArgType<std::string>::parse("") == "");
    CHECK(quikcli::ArgType<std::string>::parse("this is a string") == "this is a string");
}

TEST_CASE("CHARACTER ArgType") {
    CHECK(quikcli::ArgType<char>::parse("a") == 'a');
    CHECK(quikcli::ArgType<signed char>::parse("a") == 'a');
    CHECK(quikcli::ArgType<unsigned char>::parse("a") == 'a');
    CHECK(quikcli::ArgType<char8_t>::parse("a") == 'a');
    CHECK(quikcli::ArgType<char16_t>::parse("a") == 'a');
    CHECK(quikcli::ArgType<char32_t>::parse("a") == 'a');
    CHECK(quikcli::ArgType<wchar_t>::parse("a") == 'a');
    CHECK_THROWS_AS(quikcli::ArgType<char>::parse(""), quikcli::ParseError);
    CHECK_THROWS_AS(quikcli::ArgType<char>::parse("not a char"), quikcli::ParseError);
}

TEST_CASE("INTEGRAL ArgType") {
    CHECK(quikcli::ArgType<short>::parse("0") == 0);
    CHECK(quikcli::ArgType<int>::parse("0") == 0);
    CHECK(quikcli::ArgType<long>::parse("0") == 0);
    CHECK(quikcli::ArgType<long long>::parse("0") == 0);
    CHECK_THROWS_AS(quikcli::ArgType<int>::parse(""), quikcli::ParseError);
    CHECK_THROWS_AS(quikcli::ArgType<int>::parse("1."), quikcli::ParseError);
    CHECK_THROWS_AS(quikcli::ArgType<int>::parse("not integral"), quikcli::ParseError);
}

TEST_CASE("FLOATING_POINT ArgType") {
    CHECK(quikcli::ArgType<float>::parse("0.") == 0);
    CHECK(quikcli::ArgType<double>::parse("0.") == 0);
    CHECK(quikcli::ArgType<long double>::parse("0.") == 0);
    CHECK_THROWS_AS(quikcli::ArgType<float>::parse(""), quikcli::ParseError);
    CHECK_THROWS_AS(quikcli::ArgType<float>::parse("not floating point"), quikcli::ParseError);
}