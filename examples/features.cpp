/*
 * A feature-rich quikcli demonstration: deployment tool.
 *
 * Try:
 *   deploy cloud aws --token=abc --env=prod --region=eu-west-1 -v
 *   deploy cloud gcp --token=abc --env=staging --project=my-app -n
 *   deploy local dev api worker
 *   deploy status staging --services=api,worker
 *   deploy help
 *   deploy cloud help
 */

#include <format>
#include <iostream>
#include <optional>
#include <quikcli/command.hpp>
#include <string>
#include <vector>

using namespace quikcli;

// ─────────────────────────────────────────────────────────────────────────────
// 1. Custom ArgType
//
// A custom ArgType must fulfill the ArgTypeable concept:
//   a) Specialize std::formatter<T>       (provides std::format for displaying default values)
//   b) Specialize quikcli::ArgType<T>     (provides type hint information and parse())
// ─────────────────────────────────────────────────────────────────────────────

enum class Env { Dev, Staging, Prod };

template <> struct std::formatter<Env> : std::formatter<std::string_view> {
    auto format(Env e, std::format_context &ctx) const {
        std::string_view name;
        switch (e) {
        case Env::Dev:
            name = "dev";
            break;
        case Env::Staging:
            name = "staging";
            break;
        case Env::Prod:
            name = "prod";
            break;
        }
        return std::formatter<std::string_view>::format(name, ctx);
    }
};

template <> struct quikcli::ArgType<Env> {
    static constexpr std::string_view type_str = "ENV";
    static Env parse(std::string_view sv) {
        if (sv == "dev")
            return Env::Dev;
        if (sv == "staging")
            return Env::Staging;
        if (sv == "prod")
            return Env::Prod;
        throw quikcli::ParseError(
            std::format("'{}' is not a valid environment - expected dev, staging, or prod", sv));
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// 2. Structs use by tool
// ─────────────────────────────────────────────────────────────────────────────

struct Auth {
    std::string token;
    std::optional<std::string> username;
};
struct RunOpts {
    bool verbose;
    bool dry_run;
};

struct AwsDeploy {
    Auth auth;
    Env env;
    std::string region;
    RunOpts opts;
};
struct GcpDeploy {
    Auth auth;
    Env env;
    std::string project;
    RunOpts opts;
};
struct LocalDeploy {
    Env env;
    std::vector<std::string> services;
    RunOpts opts;
};
struct StatusQuery {
    std::optional<Env> env;
    std::vector<std::string> services;
};

// ─────────────────────────────────────────────────────────────────────────────
// 3. Flag composition into Params with custom type
//
// Flags are the building blocks of Params - a Param is one or more Flags.
// Params can be parsed into an intermediate type via operator|.
// ─────────────────────────────────────────────────────────────────────────────

// auth_param: two flags -> Auth via operator|
auto auth_param() {
    return (Flag<std::string>::required("token").doc("API authentication token") &
            Flag<std::string>::optional("username").alias('u').doc("override service account")) |
           [](std::string token, std::optional<std::string> username) {
               return Auth{std::move(token), std::move(username)};
           };
}

// run_opts_param: two flags -> RunOpts via operator|
auto run_opts_param() {
    return (Flag<bool>::no_arg("verbose").alias('v').doc("print each deployment step") &
            Flag<bool>::no_arg("dry-run").alias('n').doc("simulate without making changes")) |
           [](bool verbose, bool dry_run) { return RunOpts{verbose, dry_run}; };
}

// ─────────────────────────────────────────────────────────────────────────────
// 4. Composable Params
//
// Params themselves can also be composed. This is especially useful when
// each Param has their own intermediate types. Params without an intermediate
// type will have their types flattened when composed.
// ─────────────────────────────────────────────────────────────────────────────

// Returns Param<std::tuple<Auth, Env, RunOpts>>
auto cloud_base_param() {
    return auth_param() &
           Flag<Env>::required("env").alias('e').doc("target environment (dev|staging|prod)") &
           run_opts_param();
}

// Extends cloud_base with --region -> Param<AwsDeploy> via operator|
auto aws_param() {
    return cloud_base_param() &
               Flag<std::string>::optional_with_default("region", std::string("us-east-1"))
                   .alias('r')
                   .doc("AWS region") |
           [](Auth auth, Env env, RunOpts opts, std::string region) {
               return AwsDeploy{std::move(auth), env, std::move(region), opts};
           };
}

// Extends cloud_base with --project -> Param<GcpDeploy> via operator|
auto gcp_param() {
    return cloud_base_param() &
               Flag<std::string>::required("project").alias('p').doc("GCP project ID") |
           [](Auth auth, Env env, RunOpts opts, std::string project) {
               return GcpDeploy{std::move(auth), env, std::move(project), opts};
           };
}

// ─────────────────────────────────────────────────────────────────────────────
// 5. Basic Commands
//
// A basic Command encapsulates a Param and a function that can be applied
// to the parsed result of the Param.
// If the Param is a singular type, the function expects one argument of that type.
// If the Param is a (potentially nested) tuple (i.e. Params defined without
// operator|), the function expects n arguments where n is the arity of the
// flattened tuple.
// ─────────────────────────────────────────────────────────────────────────────

Command make_aws_cmd() {
    return Command::basic("Deploy to Amazon Web Services.", aws_param(), [](AwsDeploy d) {
        if (d.opts.dry_run)
            std::cout << "[dry-run] ";
        std::cout << std::format("Deploying to AWS {} ({})\n", d.region, d.env);
        if (d.opts.verbose && !d.auth.token.empty())
            std::cout << std::format(
                "  token: {}...\n",
                d.auth.token.substr(0, std::min<std::size_t>(6, d.auth.token.size())));
    });
}

Command make_gcp_cmd() {
    return Command::basic("Deploy to Google Cloud Platform.", gcp_param(), [](GcpDeploy d) {
        if (d.opts.dry_run)
            std::cout << "[dry-run] ";
        std::cout << std::format("Deploying project '{}' to GCP ({})\n", d.project, d.env);
        if (d.opts.verbose && !d.auth.token.empty())
            std::cout << std::format(
                "  token: {}...\n",
                d.auth.token.substr(0, std::min<std::size_t>(6, d.auth.token.size())));
    });
}

// Anonymous arguments are ordered and must be in an unambiguous ordering
Command make_local_cmd() {
    return Command::basic(
        "Deploy services to a local environment.",
        Flag<Env>::anon_optional_with_default("env", Env::Dev)
                    .doc("target environment (default: dev)") &
                Flag<std::string>::anon_variadic("services")
                    .doc("services to deploy (all if empty)") &
                run_opts_param() |
            [](Env env, std::vector<std::string> services, RunOpts opts) {
                return LocalDeploy{env, std::move(services), opts};
            },
        [](LocalDeploy d) {
            if (d.opts.dry_run)
                std::cout << "[dry-run] ";
            if (d.services.empty()) {
                std::cout << std::format("Deploying all services locally ({})\n", d.env);
            } else {
                std::cout << std::format("Deploying locally ({}):", d.env);
                for (const auto &s : d.services)
                    std::cout << ' ' << s;
                std::cout << '\n';
            }
        });
}

Command make_status_cmd() {
    auto env_services =
        Flag<Env>::anon_optional("env").doc("filter by environment") &
        Flag<std::string>::comma_delimited("services").alias('s').doc("filter by service names");

    return Command::basic(
        "Query deployment status across environments and services.",
        env_services |
            [](std::optional<Env> env, std::vector<std::string> services) {
                return StatusQuery{env, std::move(services)};
            },
        [](StatusQuery q) {
            std::cout << "=== Deployment Status ===\n";
            std::cout << "  env:      " << (q.env ? std::format("{}", *q.env) : "all") << '\n';
            if (q.services.empty()) {
                std::cout << "  services: all\n";
            } else {
                std::cout << "  services:";
                for (const auto &s : q.services)
                    std::cout << ' ' << s;
                std::cout << '\n';
            }
        });
}

// ─────────────────────────────────────────────────────────────────────────────
// 6. Nested subcommand tree
//
// Commands can then be grouped together into a subcommand tree. Each non-leaf
// subcommand is a map between the subcommand name and the command itself. The
// current structure is now:
//
//   deploy
//   ├── cloud          (group)
//   │   ├── aws        (basic)
//   │   └── gcp        (basic)
//   ├── local          (basic)
//   └── status         (basic)
// ─────────────────────────────────────────────────────────────────────────────

int main(int argc, char **argv) {
    auto cloud = Command::group("Deploy to a cloud provider.", {
                                                                   {"aws", make_aws_cmd()},
                                                                   {"gcp", make_gcp_cmd()},
                                                               });

    auto deploy = Command::group("Manage application deployments across environments.",
                                 {
                                     {"cloud", std::move(cloud)},
                                     {"local", make_local_cmd()},
                                     {"status", make_status_cmd()},
                                 });

    deploy.run(argc, argv, "2.0.0");
}
