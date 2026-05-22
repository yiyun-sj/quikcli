#pragma once
#include "flag.hpp"
#include "formatter.hpp"
#include "fwd.hpp"
#include "param.hpp"
#include "parser.hpp"

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

namespace quikcli {

class Command;

namespace detail {

inline void validate_basic_spec(const std::vector<const FlagSpec *> &specs) {
    // Duplicates are allowed for anonymous args, albeit ugly in the help message
    std::unordered_set<std::string> names{"help", "version"};
    std::unordered_set<char> aliases{'h', 'V'};
    // unambiguous anonymous args must be ordered: required -> optional/with-default -> variadic
    auto prev_anon = FlagKind::Anon;
    for (auto *s : specs) {
        if (is_anon_kind(s->kind)) {
            if (prev_anon == FlagKind::AnonVariadic)
                throw FlagError("no anonymous argument can follow a variadic one");
            if (s->kind == FlagKind::Anon && prev_anon != FlagKind::Anon)
                throw FlagError(
                    "required anonymous argument must be before optional and variadic ones");
            prev_anon = s->kind;
        } else {
            if (!names.emplace(s->long_name).second)
                throw FlagError(std::format("duplicate flag name --{}", s->long_name));
            if (s->short_alias.has_value() && !aliases.emplace(*s->short_alias).second)
                throw FlagError(std::format("duplicate flag alias -{}", *s->short_alias));
        }
    }
}

} // namespace detail

class Command {
  public:
    template <typename T, typename F>
    static Command basic(std::string summary, Param<T> param, F &&f) {
        auto specs = param.specs();
        detail::validate_basic_spec(specs);
        auto exec = [p = std::move(param), f_ = std::forward<F>(f)]() {
            std::apply(f_, detail::as_tuple(p.extract()));
        };
        return Command(Basic{std::move(summary), std::move(specs), std::move(exec)});
    }

    // Allow for implicit conversion from Flag to Param
    template <typename FlagT, typename ExtractT, typename F>
    static Command basic(std::string summary, Flag<FlagT, ExtractT> &&flag, F &&f) {
        return basic(std::move(summary), Param<ExtractT>(std::move(flag)), std::forward<F>(f));
    }

    static Command group(std::string summary,
                         std::vector<std::pair<std::string, Command>> subcommands) {
        std::unordered_set<std::string> names{"help", "version"};
        Group g;
        g.summary = std::move(summary);
        for (auto &[name, sub] : subcommands) {
            if (!names.emplace(name).second) {
                throw FlagError(std::format("duplicate subcommand name {}", name));
            }
            g.subcommands.emplace_back(std::move(name), std::make_shared<Command>(std::move(sub)));
        }
        return Command(std::move(g));
    }

    void run(int argc, char **argv, std::string_view version, std::ostream &out = std::cout,
             std::ostream &err = std::cerr) const {
        assert(argc >= 1); // argc should always contain program name when entered from cli
        RunContext ctx{argv[0], version, out, err, {}};
        run_impl(std::span<char *>(argv + 1, argc - 1), ctx);
    }

  private:
    struct Basic {
        std::string summary;
        std::vector<const FlagSpec *> specs;
        std::function<void()> run;
    };

    struct Group {
        std::string summary;
        std::vector<std::pair<std::string, std::shared_ptr<Command>>> subcommands;
    };

    using Impl = std::variant<Basic, Group>;
    Impl impl_;

    explicit Command(Impl impl) : impl_(std::move(impl)) {}

    std::string summary() const {
        return std::visit([&](const auto &cmd) { return cmd.summary; }, impl_);
    }

    struct RunContext {
        std::string_view program_path;
        std::string_view version;
        std::ostream &out;
        std::ostream &err;
        std::vector<std::string_view> subcommand_path;
    };

    void run_impl(std::span<char *> args, RunContext &ctx) const {
        std::visit([&](const auto &cmd) { run_impl(cmd, args, ctx); }, impl_);
    }

    static void run_impl(const Basic &b, std::span<char *> args, RunContext &ctx) {
        detail::Parser parser(b.specs);
        try {
            auto result = parser.parse(args);

            if (result.help_requested) {
                ctx.out << detail::Help::format_basic(ctx.program_path, b.summary,
                                                      ctx.subcommand_path, b.specs);
                return;
            }
            if (result.version_requested) {
                ctx.out << ctx.version << std::endl;
                return;
            }

            b.run();
        } catch (ParseError pe) {
            usage_info(pe.what(), ctx, true);
        }
    }

    static void run_impl(const Group &g, std::span<char *> args, RunContext &ctx) {
        if (args.empty()) {
            usage_info("missing subcommand", ctx, false);
            return;
        }
        std::string_view subcommand = args[0];
        if (subcommand == "help") {
            std::vector<std::pair<std::string, std::string>> subcommand_summaries;
            subcommand_summaries.reserve(g.subcommands.size());
            for (const auto &[name, sub] : g.subcommands) {
                subcommand_summaries.emplace_back(name, sub->summary());
            }
            ctx.out << detail::Help::format_group(ctx.program_path, g.summary, ctx.subcommand_path,
                                                  subcommand_summaries);
            return;
        }
        if (subcommand == "version") {
            ctx.out << ctx.version << std::endl;
            return;
        }
        for (const auto &[name, sub] : g.subcommands) {
            if (subcommand == name) {
                ctx.subcommand_path.emplace_back(name);
                return sub->run_impl({args.begin() + 1, args.end()}, ctx);
            }
        }
        usage_info(std::format("unknown subcommand {}", subcommand), ctx, false);
        return;
    }

    static void usage_info(std::string_view error_msg, RunContext &ctx, bool is_basic) {
        ctx.err << "Error parsing command line:\n\n  ";
        ctx.err << error_msg << "\n\n";
        ctx.err << "For usage information, run\n\n  ";
        ctx.err << ctx.program_path;
        for (auto sub : ctx.subcommand_path) {
            ctx.err << " " << sub;
        }
        ctx.err << (is_basic ? " --" : " ") << "help\n\n";
    }
};

} // namespace quikcli