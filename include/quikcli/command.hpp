#pragma once
#include "formatter.hpp"
#include "param.hpp"
#include "parser.hpp"
#include "quikcli/fwd.hpp"

#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace quikcli {

class Command {
  public:
    // TODO: check validity of param/subcommands during command construction: e.g. dups
    template <typename T, typename F>
    static Command basic(std::string summary, Param<T> param, F &&f) {
        auto specs = param.specs();
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
        Group g;
        g.summary = std::move(summary);
        for (auto &[name, sub] : subcommands)
            g.subcommands.emplace_back(std::move(name), std::make_shared<Command>(std::move(sub)));
        return Command(std::move(g));
    }

    void run(int argc, char **argv, std::string_view version, std::ostream &out = std::cout,
             std::ostream &err = std::cerr) const {
        // TODO: maybe check that argc >= 1 here: [0] should always be path
        // with an empty
        run_impl(std::span<char *>(argv + 1, argc - 1), argv[0], version, out, err);
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

    void run_impl(std::span<char *> args, std::string_view path, std::string_view version,
                  std::ostream &out = std::cout, std::ostream &err = std::cerr) const {
        std::visit([&](const auto &cmd) { run_impl(cmd, args, path, version, out, err); }, impl_);
    }

    std::string summary() const {
        return std::visit([&](const auto &cmd) { return cmd.summary; }, impl_);
    }

    static void run_impl(const Basic &b, std::span<char *> args, std::string_view path,
                         std::string_view version, std::ostream &out = std::cout,
                         std::ostream &err = std::cerr) {
        detail::Parser parser(b.specs);
        try {
            auto result = parser.parse(args);

            if (result.help_requested) {
                out << detail::Help::format_basic(path, b.summary, b.specs);
                return;
            }
            if (result.version_requested) {
                out << version << std::endl;
                return;
            }

            b.run();
        } catch (ParseError pe) {
            err << usage_info(pe.what(), path, true);
        }
    }

    static void run_impl(const Group &g, std::span<char *> args, std::string_view path,
                         std::string_view version, std::ostream &out = std::cout,
                         std::ostream &err = std::cerr) {
        if (args.empty()) {
            err << usage_info("missing subcommand", path, false);
            return;
        }
        std::string_view subcommand = args[0];
        if (subcommand == "help") {
            std::vector<std::pair<std::string, std::string>> subcommand_summaries;
            subcommand_summaries.reserve(g.subcommands.size());
            for (const auto &[name, sub] : g.subcommands) {
                subcommand_summaries.emplace_back(name, sub->summary());
            }
            out << detail::Help::format_group(path, g.summary, subcommand_summaries);
            return;
        }
        if (subcommand == "version") {
            out << version << std::endl;
            return;
        }
        for (const auto &[name, sub] : g.subcommands) {
            if (subcommand == name) {
                return sub->run_impl({args.begin() + 1, args.end()}, path, version, out, err);
            }
        }
        err << usage_info(std::format("unknown subcommand {}", subcommand), path, false);
        return;
    }

    // TODO: usage_info and help message both need all parent subcommands
    static std::string usage_info(std::string_view error_msg, std::string_view path,
                                  bool is_basic) {
        std::string out;
        out += "Error parsing command line:\n\n  ";
        out += error_msg;
        out += "\n\n";
        out += "For usage information, run\n\n  ";
        out += path;
        out += is_basic ? " --" : " ";
        out += "help\n\n";
        return out;
    }
};

} // namespace quikcli