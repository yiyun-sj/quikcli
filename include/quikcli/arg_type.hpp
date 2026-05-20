#pragma once
#include "fwd.hpp"

#include <charconv>
#include <concepts>
#include <format>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace quikcli {

namespace detail {

template <typename T>
concept Roundtrippable = requires(std::string_view sv, T val) {
    { ArgType<T>::type_str } -> std::convertible_to<std::string_view>;
    { ArgType<T>::parse(sv) } -> std::same_as<T>;
};

template <typename T>
concept Character =
    std::same_as<T, char> || std::same_as<T, signed char> || std::same_as<T, unsigned char> ||
    std::same_as<T, char8_t> || std::same_as<T, char16_t> || std::same_as<T, char32_t> ||
    std::same_as<T, wchar_t>;

} // namespace detail

template <> struct ArgType<std::string> {
    static constexpr std::string_view type_str = "STRING";
    static std::string parse(std::string_view sv) { return std::string(sv); };
    static std::string to_string(std::string val) { return val; };
};

template <> struct ArgType<bool> {
    static constexpr std::string_view type_str = "BOOL";
    static bool parse(std::string_view sv) {
        if (sv == "true")
            return true;
        if (sv == "false")
            return false;
        throw ParseError(std::format("boolean arguments expects true or false, got {}", sv));
    };
};

template <typename T>
    requires(detail::Character<T>)
struct ArgType<T> {
    static constexpr std::string_view type_str = "CHAR";
    static T parse(std::string_view sv) {
        if (sv.size() != 1)
            throw ParseError(
                std::format("character arguments expects a single character, got {}", sv));
        return static_cast<T>(sv[0]);
    }
};

template <typename T>
    requires(std::integral<T> && !std::same_as<T, bool> && !detail::Character<T>)
struct ArgType<T> {
    static constexpr std::string_view type_str = "INT";
    static T parse(std::string_view sv) {
        T value{};
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
        if (ec != std::errc{} || ptr != sv.data() + sv.size())
            throw ParseError(std::format("integer argument failed to parse, got {}", sv));
        return value;
    }
};

template <typename T>
    requires std::floating_point<T>
struct ArgType<T> {
    static constexpr std::string_view type_str = "FLOAT";
    static T parse(std::string_view sv) {
        T value{};
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
        if (ec != std::errc{} || ptr != sv.data() + sv.size())
            throw ParseError(std::format("float argument failed to parse, got {}", sv));
        return value;
    }
};

} // namespace quikcli