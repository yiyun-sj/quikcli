#pragma once
#include "arg_type.hpp"
#include "fwd.hpp"

#include <concepts>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace quikcli {

struct FlagNameError : std::invalid_argument {
    explicit FlagNameError(std::string msg) : std::invalid_argument(std::move(msg)) {}
};

namespace detail {

inline void validate_flag_name(std::string_view name) {
    if (name.empty())
        throw FlagNameError("Flag name cannot be empty");
    if (name.front() == '-')
        throw FlagNameError("Flag name cannot start with a '-'");
}

} // namespace detail

enum class FlagKind { Required, Optional, OptionalWithDefault, NoArg };

struct FlagSpec {
    std::string long_name;
    std::string doc_str;
    std::optional<char> short_alias;
    FlagKind kind;
    mutable std::optional<std::string> raw_value;
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

    Flag<T> &&doc(std::string text) && {
        spec_.doc_str = std::move(text);
        return std::move(*this);
    }

    Flag<T> &&alias(char c) && {
        spec_.short_alias = c;
        return std::move(*this);
    }

    const FlagSpec &spec() const { return spec_; }
    std::optional<T> default_value() const { return default_; }

  private:
    template <typename U> friend class Flag;
    template <typename U> friend class Param;

    FlagSpec spec_;
    std::optional<T> default_;

    explicit Flag(FlagSpec s) : spec_(std::move(s)) {}
};

} // namespace quikcli
