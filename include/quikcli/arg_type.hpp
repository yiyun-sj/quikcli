#pragma once
#include "fwd.hpp"

#include <charconv>
#include <concepts>
#include <format>
#include <string>
#include <string_view>

namespace quikcli {

template <typename T> struct ArgType {
    static_assert(sizeof(T) == 0, "ArgType<T>: no specialization exists for type T. "
                                  "Use a supported built-in type or provide a specialization.");
};

template <> struct ArgType<std::string> {
    static std::string parse(std::string_view sv) { return std::string(sv); };
};

template <> struct ArgType<bool> {
    static bool parse(std::string_view sv) {
        if (sv == "true")
            return true;
        if (sv == "false")
            return false;
        throw ParseError(std::format("BOOL arguments expects true or false, got {}", sv));
    };
};

template <typename T>
concept Character =
    std::same_as<T, char> || std::same_as<T, signed char> || std::same_as<T, unsigned char> ||
    std::same_as<T, char8_t> || std::same_as<T, char16_t> || std::same_as<T, char32_t> ||
    std::same_as<T, wchar_t>;

template <typename T>
    requires(Character<T>)
struct ArgType<T> {
    static T parse(std::string_view sv) {
        if (sv.size() != 1)
            throw ParseError(
                std::format("CHARACTER arguments expects a single character, got {}", sv));
        return static_cast<T>(sv[0]);
    }
};

template <typename T>
    requires(std::integral<T> && !std::same_as<T, bool> && !Character<T>)
struct ArgType<T> {
    static T parse(std::string_view sv) {
        T value{};
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
        if (ec != std::errc{} || ptr != sv.data() + sv.size())
            throw ParseError(std::format("INTEGRAL argument failed to parse, got {}", sv));
        return value;
    }
};

template <typename T>
    requires std::floating_point<T>
struct ArgType<T> {
    static T parse(std::string_view sv) {
        T value{};
        auto [ptr, ec] = std::from_chars(sv.data(), sv.data() + sv.size(), value);
        if (ec != std::errc{} || ptr != sv.data() + sv.size())
            throw ParseError(std::format("FLOATING_POINT argument failed to parse, got {}", sv));
        return value;
    }
};

} // namespace quikcli