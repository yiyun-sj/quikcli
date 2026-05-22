#pragma once
#include "arg_type.hpp"
#include "fwd.hpp"

#include <concepts>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace quikcli {

struct FlagError : std::invalid_argument {
    explicit FlagError(std::string msg) : std::invalid_argument(std::move(msg)) {}
};

namespace detail {

template <typename T> struct is_optional : std::false_type {};
template <typename T> struct is_optional<std::optional<T>> : std::true_type {};
template <typename T> inline constexpr bool is_optional_v = is_optional<T>::value;

template <typename T> struct is_vector : std::false_type {};
template <typename T> struct is_vector<std::vector<T>> : std::true_type {};
template <typename T> inline constexpr bool is_vector_v = is_vector<T>::value;

inline void validate_flag_name(std::string_view name) {
    if (name.empty())
        throw FlagError("flag name cannot be empty");
    if (name.front() == '-')
        throw FlagError("flag name cannot start with a '-'");
    if (name.find('=') != std::string_view::npos)
        throw FlagError("flag name cannot contain '='");
}

inline void validate_flag_alias(char alias) {
    if (alias == '-')
        throw FlagError("flag alias cannot be '-'");
}

} // namespace detail

enum class FlagKind {
    Required,
    Optional,
    OptionalWithDefault,
    NoArg,
    CommaDelimited,
    Anon,
    AnonOptional,
    AnonOptionalWithDefault,
    AnonVariadic
};

inline bool is_anon_kind(FlagKind k) {
    return k == FlagKind::Anon || k == FlagKind::AnonOptional ||
           k == FlagKind::AnonOptionalWithDefault || k == FlagKind::AnonVariadic;
}

struct FlagSpec {
    std::string long_name;
    std::string type_hint;
    std::string doc_str;
    std::string default_str;
    std::optional<char> short_alias;
    FlagKind kind;
    mutable std::optional<std::string> raw_value;
    mutable std::vector<std::string> raw_values; // Special case for AnonVariadic
};

template <typename T, typename ExtractT = T> class Flag {
  public:
    static Flag<T> required(std::string name)
        requires detail::Roundtrippable<T>
    {
        detail::validate_flag_name(name);
        FlagSpec s;
        s.long_name = std::move(name);
        s.type_hint = ArgType<T>::type_str;
        s.kind = FlagKind::Required;
        return Flag<T>(std::move(s));
    }

    static Flag<T, std::optional<T>> optional(std::string name)
        requires detail::Roundtrippable<T>
    {
        detail::validate_flag_name(name);
        FlagSpec s;
        s.long_name = std::move(name);
        s.type_hint = ArgType<T>::type_str;
        s.kind = FlagKind::Optional;
        return Flag<T, std::optional<T>>(std::move(s));
    }

    static Flag<T> optional_with_default(std::string name, T default_value)
        requires detail::Roundtrippable<T>
    {
        detail::validate_flag_name(name);
        FlagSpec s;
        s.long_name = std::move(name);
        s.type_hint = ArgType<T>::type_str;
        s.default_str = std::format("{}", default_value);
        s.kind = FlagKind::OptionalWithDefault;
        Flag<T> f(std::move(s));
        f.default_ = std::move(default_value);
        return f;
    }

    static Flag<T> no_arg(std::string name)
        requires std::same_as<T, bool>
    {
        detail::validate_flag_name(name);
        FlagSpec s;
        s.long_name = std::move(name);
        s.type_hint = ArgType<T>::type_str;
        s.kind = FlagKind::NoArg;
        return Flag<T>(std::move(s));
    }

    static Flag<T, std::vector<T>> comma_delimited(std::string name)
        requires detail::Roundtrippable<T>
    {
        detail::validate_flag_name(name);
        FlagSpec s;
        s.long_name = std::move(name);
        s.type_hint = ArgType<T>::type_str;
        s.kind = FlagKind::CommaDelimited;
        return Flag<T, std::vector<T>>(std::move(s));
    }

    static Flag<T> anon(std::string name)
        requires detail::Roundtrippable<T>
    {
        FlagSpec s;
        s.long_name = std::move(name);
        s.type_hint = ArgType<T>::type_str;
        s.kind = FlagKind::Anon;
        return Flag<T>(std::move(s));
    }

    static Flag<T, std::optional<T>> anon_optional(std::string name)
        requires detail::Roundtrippable<T>
    {
        FlagSpec s;
        s.long_name = std::move(name);
        s.type_hint = ArgType<T>::type_str;
        s.kind = FlagKind::AnonOptional;
        return Flag<T, std::optional<T>>(std::move(s));
    }

    static Flag<T> anon_optional_with_default(std::string name, T default_value)
        requires detail::Roundtrippable<T>
    {
        FlagSpec s;
        s.long_name = std::move(name);
        s.type_hint = ArgType<T>::type_str;
        s.default_str = std::format("{}", default_value);
        s.kind = FlagKind::AnonOptionalWithDefault;
        Flag<T> f(std::move(s));
        f.default_ = std::move(default_value);
        return f;
    }

    static Flag<T, std::vector<T>> anon_variadic(std::string name)
        requires detail::Roundtrippable<T>
    {
        FlagSpec s;
        s.long_name = std::move(name);
        s.type_hint = ArgType<T>::type_str;
        s.kind = FlagKind::AnonVariadic;
        return Flag<T, std::vector<T>>(std::move(s));
    }

    Flag<T, ExtractT> &&doc(std::string text) && {
        spec_.doc_str = std::move(text);
        return std::move(*this);
    }

    Flag<T, ExtractT> &&alias(char c) && {
        detail::validate_flag_alias(c);
        spec_.short_alias = c;
        return std::move(*this);
    }

    // Special case for optional and variadic flags
    ExtractT extract()
        requires(!std::same_as<T, ExtractT>)
    {
        switch (spec_.kind) {
        case FlagKind::Optional:
        case FlagKind::AnonOptional:
            if constexpr (detail::is_optional_v<ExtractT>) {
                if (!spec_.raw_value.has_value())
                    return std::nullopt;
                return ArgType<T>::parse(*spec_.raw_value);
            }
            // Optional is optional<U> by construction
            throw ParseError("BUG: Optional flag is not a optional<U>");
        case FlagKind::CommaDelimited: {
            if constexpr (detail::is_vector_v<ExtractT>) {
                auto parts =
                    *spec_.raw_value | std::views::split(',') | std::views::transform([](auto &&r) {
                        return std::string_view(r.begin(), r.end());
                    });

                ExtractT result;
                for (auto part : parts)
                    result.push_back(ArgType<T>::parse(part));
                return result;
            }
            throw ParseError("BUG: CommaDelimited flag is not vector<U>");
        }
        case FlagKind::AnonVariadic:
            if constexpr (detail::is_vector_v<ExtractT>) {
                ExtractT result;
                for (const auto &s : spec_.raw_values)
                    result.push_back(ArgType<T>::parse(s));
                return result;
            }
            throw ParseError("BUG: AnonVariadic flag is not vector<U>");
        default:
            throw ParseError("BUG: only optional and variadic flags can have T != ExtractT");
        }
    }

    ExtractT extract()
        requires std::same_as<T, ExtractT>
    {
        switch (spec_.kind) {
        case FlagKind::NoArg:
            if constexpr (std::same_as<bool, T>) {
                return spec_.raw_value.has_value();
            }
            // NoArg is required to be bool
            throw ParseError("BUG: NoArg flag is not bool");
        case FlagKind::Required:
            if (!spec_.raw_value.has_value())
                throw ParseError(std::format("required flag --{} missing", spec_.long_name));
            return ArgType<T>::parse(*spec_.raw_value);
        case FlagKind::OptionalWithDefault:
        case FlagKind::AnonOptionalWithDefault:
            if (spec_.raw_value.has_value())
                return ArgType<T>::parse(*spec_.raw_value);
            return *default_;
        case FlagKind::Anon:
            if (!spec_.raw_value.has_value())
                throw ParseError(std::format("required anonymous argument missing"));
            return ArgType<T>::parse(*spec_.raw_value);
        default:
            throw ParseError("BUG: optional and variadic flags must have T != ExtractT");
        }
    }

    const FlagSpec &spec() const { return spec_; }
    std::optional<T> default_value() const { return default_; }

  private:
    template <typename U, typename ExtractU> friend class Flag;
    template <typename U> friend class Param;

    FlagSpec spec_;
    std::optional<T> default_;

    explicit Flag(FlagSpec s) : spec_(std::move(s)) {}
};

} // namespace quikcli
