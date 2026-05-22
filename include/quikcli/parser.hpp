#pragma once
#include "flag.hpp"

#include <cassert>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace quikcli {
namespace detail {

enum class TokenKind { LongFlag, ShortFlag, ValueOrPositional };

struct Token {
    TokenKind kind;
    std::string text;
};

struct ParseResult {
    bool help_requested = false;
    bool version_requested = false;
};

class Parser {
  public:
    explicit Parser(std::vector<const FlagSpec *> specs) {
        for (auto *s : specs) {
            // raw value is cleared because FlagSpec can be shared since Param and Command both use
            // shared_ptrs; as long as parsing doesn't run in parallel, this should be okay
            s->raw_value = std::nullopt;
            s->raw_values.clear();

            bool is_anon = s->kind == FlagKind::Anon || s->kind == FlagKind::AnonOptional ||
                           s->kind == FlagKind::AnonOptionalWithDefault ||
                           s->kind == FlagKind::AnonVariadic;
            if (is_anon) {
                anon_specs_.push_back(s);
            } else {
                name_to_flag_spec_.emplace(s->long_name, s);
                if (s->short_alias.has_value()) {
                    alias_to_flag_spec_.emplace(*s->short_alias, s);
                }
            }
        }
    }

    ParseResult parse(std::span<char *> args) const {
        auto tokens = tokenize(args);
        ParseResult out;
        match_tokens(tokens, out);
        return out;
    }

  private:
    std::vector<Token> tokenize(std::span<char *> args) const {
        std::vector<Token> tokens;
        bool past_double_dash = false;

        for (std::string_view arg : args) {

            if (past_double_dash) {
                tokens.push_back({TokenKind::ValueOrPositional, std::string(arg)});
                continue;
            }

            if (arg == "--") {
                past_double_dash = true;
                continue;
            }

            if (arg.size() > 2 && arg[0] == '-' && arg[1] == '-') {
                auto rest = arg.substr(2);
                auto eq = rest.find('=');
                if (eq != std::string_view::npos) {
                    tokens.push_back({TokenKind::LongFlag, std::string(rest.substr(0, eq))});
                    tokens.push_back(
                        {TokenKind::ValueOrPositional, std::string(rest.substr(eq + 1))});
                } else {
                    tokens.push_back({TokenKind::LongFlag, std::string(rest)});
                }
                continue;
            }

            if (arg.size() >= 2 && arg[0] == '-') {
                auto chars = arg.substr(1);
                for (std::size_t j = 0; j < chars.size(); ++j) {
                    char c = chars[j];
                    const FlagSpec *spec = find_short(c);
                    bool is_last = (j == chars.size() - 1);
                    bool is_no_arg = (spec == nullptr || spec->kind == FlagKind::NoArg);

                    if (is_no_arg || is_last) {
                        tokens.push_back({TokenKind::ShortFlag, std::string(1, c)});
                    } else {
                        tokens.push_back({TokenKind::ShortFlag, std::string(1, c)});
                        tokens.push_back(
                            {TokenKind::ValueOrPositional, std::string(chars.substr(j + 1))});
                        break;
                    }
                }
                continue;
            }

            tokens.push_back({TokenKind::ValueOrPositional, std::string(arg)});
        }

        return tokens;
    }

    void match_tokens(const std::vector<Token> &tokens, ParseResult &out) const {
        for (const auto &tok : tokens) {
            if ((tok.kind == TokenKind::LongFlag && tok.text == "help") ||
                (tok.kind == TokenKind::ShortFlag && tok.text == "h")) {
                out.help_requested = true;
            }
            if ((tok.kind == TokenKind::LongFlag && tok.text == "version") ||
                (tok.kind == TokenKind::ShortFlag && tok.text == "V")) {
                out.version_requested = true;
            }
        }
        if (out.help_requested || out.version_requested)
            return;

        std::vector<std::string> positionals;

        for (std::size_t i = 0; i < tokens.size(); ++i) {
            const auto &tok = tokens[i];

            if (tok.kind == TokenKind::ValueOrPositional) {
                positionals.push_back(tok.text);
                continue;
            }

            const FlagSpec *spec = nullptr;
            std::string flag_str;

            if (tok.kind == TokenKind::LongFlag) {
                spec = find_long(tok.text);
                flag_str = "--" + tok.text;
            } else {
                spec = find_short(tok.text[0]);
                flag_str = "-" + tok.text;
            }

            if (!spec)
                throw ParseError(std::format("unknown flag {}", flag_str));

            if (spec->raw_value.has_value())
                throw ParseError(std::format("duplicate flag {}", flag_str));

            if (spec->kind == FlagKind::NoArg) {
                spec->raw_value = "";
                continue;
            }

            if (i + 1 < tokens.size() && tokens[i + 1].kind == TokenKind::ValueOrPositional) {
                spec->raw_value = tokens[i + 1].text;
                ++i;
            } else {
                throw ParseError(std::format("flag {} requires a value", flag_str));
            }
        }

        // Match positionals against anon specs in declaration order
        std::size_t pos_idx = 0;
        for (auto *anon : anon_specs_) {
            if (anon->kind == FlagKind::AnonVariadic) {
                for (; pos_idx < positionals.size(); ++pos_idx)
                    anon->raw_values.push_back(positionals[pos_idx]);
                break;
            }
            if (pos_idx < positionals.size())
                anon->raw_value = positionals[pos_idx++];
        }
        if (pos_idx != positionals.size()) {
            throw ParseError("too many anonymous arguments");
        }
    }

    const FlagSpec *find_long(std::string_view name) const {
        auto it = name_to_flag_spec_.find(std::string(name));
        if (it != name_to_flag_spec_.end()) {
            return it->second;
        }
        return nullptr;
    }

    const FlagSpec *find_short(char alias) const {
        auto it = alias_to_flag_spec_.find(alias);
        if (it != alias_to_flag_spec_.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::unordered_map<std::string, const FlagSpec *> name_to_flag_spec_;
    std::unordered_map<char, const FlagSpec *> alias_to_flag_spec_;
    std::vector<const FlagSpec *> anon_specs_;
};

} // namespace detail
} // namespace quikcli
