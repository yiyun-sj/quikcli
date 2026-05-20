#pragma once
#include "arg_type.hpp"
#include "fwd.hpp"

#include <concepts>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace quikcli {

struct FlagNameError : std::invalid_argument {
    explicit FlagNameError(std::string msg) : std::invalid_argument(std::move(msg)) {}
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
        throw FlagNameError("Flag name cannot be empty");
    if (name.front() == '-')
        throw FlagNameError("Flag name cannot start with a '-'");
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

struct FlagSpec {
    std::string long_name;
    std::string doc_str;
    std::optional<char> short_alias;
    FlagKind kind;
    mutable std::optional<std::string> raw_value;
    mutable std::vector<std::string> raw_values; // Special case for AnonVariadic
};

template <typename T> class Flag {
  public:
    static Flag<T> required(std::string name)
        requires detail::Parseable<T>
    {
        detail::validate_flag_name(name);
        FlagSpec s;
        s.long_name = std::move(name);
        s.kind = FlagKind::Required;
        return Flag<T>(std::move(s));
    }

    static Flag<std::optional<T>> optional(std::string name)
        requires detail::Parseable<T>
    {
        detail::validate_flag_name(name);
        FlagSpec s;
        s.long_name = std::move(name);
        s.kind = FlagKind::Optional;
        return Flag<std::optional<T>>(std::move(s));
    }

    static Flag<T> optional_with_default(std::string name, T default_value)
        requires detail::Parseable<T>
    {
        detail::validate_flag_name(name);
        FlagSpec s;
        s.long_name = std::move(name);
        s.kind = FlagKind::OptionalWithDefault;
        Flag<T> f(std::move(s));
        f.default_ = std::move(default_value);
        return f;
    }

    static Flag<T> no_arg(std::string name)
        requires std::same_as<T, bool>
    {
        FlagSpec s;
        s.long_name = std::move(name);
        s.kind = FlagKind::NoArg;
        return Flag<T>(std::move(s));
    }

    static Flag<std::vector<T>> comma_delimited(std::string name)
        requires detail::Parseable<T>
    {
        detail::validate_flag_name(name);
        FlagSpec s;
        s.long_name = std::move(name);
        s.kind = FlagKind::CommaDelimited;
        return Flag<std::vector<T>>(std::move(s));
    }

    static Flag<T> anon()
        requires detail::Parseable<T>
    {
        FlagSpec s;
        s.kind = FlagKind::Anon;
        return Flag<T>(std::move(s));
    }

    static Flag<std::optional<T>> anon_optional()
        requires detail::Parseable<T>
    {
        FlagSpec s;
        s.kind = FlagKind::AnonOptional;
        return Flag<std::optional<T>>(std::move(s));
    }

    static Flag<T> anon_optional_with_default(T default_value)
        requires detail::Parseable<T>
    {
        FlagSpec s;
        s.kind = FlagKind::AnonOptionalWithDefault;
        Flag<T> f(std::move(s));
        f.default_ = std::move(default_value);
        return f;
    }

    static Flag<std::vector<T>> anon_variadic()
        requires detail::Parseable<T>
    {
        FlagSpec s;
        s.kind = FlagKind::AnonVariadic;
        return Flag<std::vector<T>>(std::move(s));
    }

    Flag<T> &&doc(std::string text) && {
        spec_.doc_str = std::move(text);
        return std::move(*this);
    }

    Flag<T> &&alias(char c) && {
        spec_.short_alias = c;
        return std::move(*this);
    }

    T extract() {
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
        case FlagKind::Optional:
        case FlagKind::AnonOptional:
            if constexpr (detail::is_optional_v<T>) {
                if (!spec_.raw_value.has_value())
                    return std::nullopt;
                return ArgType<typename T::value_type>::parse(*spec_.raw_value);
            }
            // Optional is optional<U> by construction
            throw ParseError("BUG: Optional flag is not a optional<U>");
        case FlagKind::CommaDelimited:
            return ArgType<T>::parse(spec_.raw_value.value_or(""));
        case FlagKind::Anon:
            if (!spec_.raw_value.has_value())
                throw ParseError(std::format("required anonymous argument missing"));
            return ArgType<T>::parse(*spec_.raw_value);
        case FlagKind::AnonVariadic:
            if constexpr (detail::is_vector_v<T>) {
                T result;
                for (const auto &s : spec_.raw_values)
                    result.push_back(ArgType<typename T::value_type>::parse(s));
                return result;
            }
            throw ParseError("BUG: AnonVariadic flag is not vector<U>");
        }
    }

    const FlagSpec &spec() const { return spec_; }
    std::optional<T> default_value() const { return default_; }

  private:
    template <typename U> friend class Flag;
    template <typename... Us> friend class Param;

    FlagSpec spec_;
    std::optional<T> default_;

    explicit Flag(FlagSpec s) : spec_(std::move(s)) {}
};

} // namespace quikcli
