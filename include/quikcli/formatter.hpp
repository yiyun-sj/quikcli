#pragma once
#include "flag.hpp"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace quikcli {
namespace detail {

class HelpFormatter {
  public:
    static std::string format_basic(std::string_view program_path, std::string_view summary,
                                    const std::vector<const FlagSpec *> &specs) {
        std::vector<const FlagSpec *> flag_specs, anon_specs;
        for (auto *s : specs) {
            if (is_anon_kind(s->kind))
                anon_specs.push_back(s);
            else
                flag_specs.push_back(s);
        }

        std::string out;

        out += summary;
        out += "\n\n";

        out += "  ";
        out += basename_of(program_path);
        for (auto *s : anon_specs) {
            out += " ";
            out += anon_usage_str(s);
        }
        out += "\n\n";

        auto args_msg = args_table(anon_specs);
        if (!args_msg.empty()) {
            out += "=== arguments ===\n\n";
            out += args_msg;
            out += "\n";
        }

        out += "=== flags ===\n\n";

        out += flags_table(flag_specs);

        return out;
    }

    static std::string
    format_group(std::string_view program_path, std::string_view summary,
                 const std::vector<std::pair<std::string, std::string>> &subcommand_summaries) {
        std::string out;

        out += summary;
        out += "\n\n";

        out += "  ";
        out += basename_of(program_path);
        out += " <subcommand>\n\n";

        out += "=== subcommands ===\n\n";

        out += subcommands_table(subcommand_summaries);

        return out;
    }

  private:
    struct Row {
        std::string inputs, doc;
    };

    static std::string args_table(const std::vector<const FlagSpec *> &specs) {
        std::vector<Row> rows;
        for (auto *s : specs)
            rows.push_back({build_arg_inputs(*s), build_annotated_doc(*s)});
        return render_rows(rows);
    }

    static std::string flags_table(const std::vector<const FlagSpec *> &specs) {
        std::vector<Row> rows;
        for (auto *s : specs)
            rows.push_back({build_flag_inputs(*s), build_annotated_doc(*s)});
        rows.push_back({"[-V, --version]", "print the version and exit"});
        rows.push_back({"[-h, --help]", "print this help text and exit"});
        return render_rows(rows);
    }

    static std::string subcommands_table(
        const std::vector<std::pair<std::string, std::string>> &subcommand_summaries) {
        std::vector<Row> rows;
        for (const auto &s : subcommand_summaries)
            rows.push_back({s.first, s.second});
        rows.push_back({"version", "print version information"});
        rows.push_back({"help", "explain a given subcommand"});
        return render_rows(rows);
    }

    static std::string render_rows(const std::vector<Row> &rows) {
        int max_left = 0;
        for (const auto &r : rows)
            max_left = std::max(max_left, (int)r.inputs.size());

        constexpr int tab = 2;
        std::string out;
        for (const auto &r : rows) {
            out += std::string(tab, ' ');
            out += r.inputs;
            out += std::string(max_left - (int)r.inputs.size() + tab, ' ');
            out += ". " + r.doc;
            out += "\n";
        }
        return out;
    }

    static bool is_anon_kind(FlagKind k) {
        return k == FlagKind::Anon || k == FlagKind::AnonOptional ||
               k == FlagKind::AnonOptionalWithDefault || k == FlagKind::AnonVariadic;
    }

    static std::string build_annotated_doc(const FlagSpec &s) {
        switch (s.kind) {
        case FlagKind::AnonOptionalWithDefault:
        case FlagKind::OptionalWithDefault:
            return s.doc_str + "(default: " + s.default_str + ")";
        default:
            return s.doc_str;
        }
    }

    static std::string build_arg_inputs(const FlagSpec &s) {
        switch (s.kind) {
        case FlagKind::Anon:
        case FlagKind::AnonOptional:
        case FlagKind::AnonOptionalWithDefault:
        case FlagKind::AnonVariadic:
            return s.long_name + "=" + s.type_hint;
        default:
            return "";
        }
    }

    static std::string build_flag_inputs(const FlagSpec &s) {
        std::string long_flag = "--" + s.long_name;
        std::string short_flag =
            s.short_alias.has_value() ? "-" + std::string{*s.short_alias} + ", " : "    ";
        switch (s.kind) {
        case FlagKind::Required:
            return " " + short_flag + long_flag + "=" + s.type_hint + " ";
        case FlagKind::NoArg:
            return "[" + short_flag + long_flag + "]";
        case FlagKind::Optional:
        case FlagKind::OptionalWithDefault:
            return "[" + short_flag + long_flag + "=" + s.type_hint + "]";
        case FlagKind::CommaDelimited:
            return "[" + short_flag + long_flag + "=" + s.type_hint + "] ...";
        default:
            return "";
        }
    }

    static std::string anon_usage_str(const FlagSpec *s) {
        switch (s->kind) {
        case FlagKind::Anon:
            return "<" + s->long_name + ">";
        case FlagKind::AnonOptional:
        case FlagKind::AnonOptionalWithDefault:
            return "[" + s->long_name + "]";
        case FlagKind::AnonVariadic:
            return "[" + s->long_name + " ...]";
        default:
            return "";
        }
    }

    static std::string basename_of(std::string_view path) {
        auto pos = path.rfind('/');
        return std::string(pos != std::string_view::npos ? path.substr(pos + 1) : path);
    }
};

} // namespace detail
} // namespace quikcli
