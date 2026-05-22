#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <quikcli/arg_type.hpp>

TEST_CASE("bool argtype") {
    CHECK(quikcli::ArgType<bool>::parse("true") == true);
    CHECK(quikcli::ArgType<bool>::parse("false") == false);
    CHECK_THROWS_AS(quikcli::ArgType<bool>::parse("anything else"), quikcli::ParseError);
}

TEST_CASE("string argtype") {
    CHECK(quikcli::ArgType<std::string>::parse("") == "");
    CHECK(quikcli::ArgType<std::string>::parse("this is a string") == "this is a string");
}

TEST_CASE("character argtype") {
    CHECK(quikcli::ArgType<char>::parse("a") == 'a');
    CHECK(quikcli::ArgType<signed char>::parse("a") == 'a');
    CHECK(quikcli::ArgType<unsigned char>::parse("a") == 'a');
    CHECK_THROWS_AS(quikcli::ArgType<char>::parse(""), quikcli::ParseError);
    CHECK_THROWS_AS(quikcli::ArgType<char>::parse("not a char"), quikcli::ParseError);
}

TEST_CASE("integer argtype") {
    CHECK(quikcli::ArgType<short>::parse("0") == 0);
    CHECK(quikcli::ArgType<int>::parse("0") == 0);
    CHECK(quikcli::ArgType<long>::parse("0") == 0);
    CHECK(quikcli::ArgType<long long>::parse("0") == 0);
    CHECK_THROWS_AS(quikcli::ArgType<int>::parse(""), quikcli::ParseError);
    CHECK_THROWS_AS(quikcli::ArgType<int>::parse("1."), quikcli::ParseError);
    CHECK_THROWS_AS(quikcli::ArgType<int>::parse("not integral"), quikcli::ParseError);
}

TEST_CASE("float argtype") {
    CHECK(quikcli::ArgType<float>::parse("0.") == 0);
    CHECK(quikcli::ArgType<double>::parse("0.") == 0);
    CHECK(quikcli::ArgType<long double>::parse("0.") == 0);
    CHECK_THROWS_AS(quikcli::ArgType<float>::parse(""), quikcli::ParseError);
    CHECK_THROWS_AS(quikcli::ArgType<float>::parse("not floating point"), quikcli::ParseError);
}

enum class Color { Red, Green, Blue };

template <> struct quikcli::ArgType<Color> {
    static constexpr std::string_view type_str = "COLOR";
    static Color parse(std::string_view sv) {
        if (sv == "red")
            return Color::Red;
        if (sv == "green")
            return Color::Green;
        if (sv == "blue")
            return Color::Blue;
        throw ParseError(std::format("color arguments expects red, green, or blue, got {}", sv));
    };
};

TEST_CASE("custom argtype") {
    CHECK(quikcli::ArgType<Color>::parse("red") == Color::Red);
    CHECK(quikcli::ArgType<Color>::parse("green") == Color::Green);
    CHECK(quikcli::ArgType<Color>::parse("blue") == Color::Blue);
    CHECK_THROWS_AS(quikcli::ArgType<Color>::parse("anything else"), quikcli::ParseError);
}