/*
 * Copyright (c) 2026 Yiyun Jia
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <charconv>
#include <concepts>
#include <format>
#include <string>
#include <string_view>

namespace quikcli {

template <typename T> struct ArgType;

namespace detail {

// Backport of C++ 23 Formattable concept; used to print default values of the arg.
template <typename T>
concept Formattable = requires(T &v, std::format_context ctx) {
    std::formatter<std::remove_cvref_t<T>>().format(v, ctx);
};

template <typename T>
concept ArgTypeable = requires(std::string_view sv, T val) {
    { ArgType<T>::type_str } -> std::convertible_to<std::string_view>;
    { ArgType<T>::parse(sv) } -> std::same_as<T>;
} && Formattable<T>;

template <typename T>
concept Integer =
    std::integral<T> &&
    !(std::same_as<T, bool> || std::same_as<T, char> || std::same_as<T, signed char> ||
      std::same_as<T, unsigned char> || std::same_as<T, char8_t> || std::same_as<T, char16_t> ||
      std::same_as<T, char32_t> || std::same_as<T, wchar_t>);

} // namespace detail

struct ParseError : std::runtime_error {
    explicit ParseError(const std::string &msg) : std::runtime_error(msg) {}
};

template <> struct ArgType<std::string> {
    static constexpr std::string_view type_str = "STRING";
    static std::string parse(std::string_view sv) { return std::string(sv); };
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
    requires(std::same_as<T, char> || std::same_as<T, signed char> ||
             std::same_as<T, unsigned char>)
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
    requires(detail::Integer<T>)
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