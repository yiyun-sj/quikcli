# QuikCli

QuikCli is an opinionated header-only command-line interface builder for C++20.

Heavily inspired by OCaml's [Core.Command](https://ocaml.org/p/core/v0.14.1/doc/core/Core/Command/index.html), it brings a similar compositional ergonomic to C++. Type safety is maintained at all levels.

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [API Reference](#api-reference)
  - [ArgType\<T\>](#argtypet)
  - [Flag\<T\>](#flagt)
  - [Param\<T\>](#paramt)
  - [Command](#command)
- [Build & Test](#build--test)

---

## Installation

This is a header-only library. You can drop in the files from [`include/quikcli`](https://github.com/yiyun-sj/quikcli/tree/main/include/quikcli) to your project and begin using the library with

```cpp
#include <quikcli/quikcli.hpp>
```

---

## Quick Start

See the [examples folder](https://github.com/yiyun-sj/quikcli/tree/main/examples) for examples to get started. The `basic.cpp` example shows simple usage and the `features.cpp` tries to give a rundown of (most of) what you can do with this library. The following is a short snippet showing basic usage:

```cpp
auto cmd = Command::basic(
    "a summary of the command",

    Flag<std::string>::required("filename").doc("the file to read")
        & Flag<bool>::no_arg("verbose").alias('v').doc("show verbose messages")
        & Flag<int>::optional_with_default("count", 10).alias('n')
                .doc("number of words"),

    [](std::string filename, bool verbose, int count) {
        // Do something with the flags
    });

cmd.run(argc, argv, "version 1.0.0");
```

---

## Feature Reference

### `ArgType<T>`

`ArgType<T>` defines how to parse raw strings into type `T`. Custom types are supported and can be implemented via fulfilling the `ArgTypeable` concept.

**Built-in specializations**

| C++ type | CLI type hint | Accepted values |
|---|---|---|
| `std::string` | `STRING` | any string |
| `bool` | `BOOL` | `true` / `false` |
| `char`, `signed char`, `unsigned char` | `CHAR` | a single character |
| integer types (`short`, `int`, `long`, …) | `INT` | decimal integer |
| floating-point types (`float`, `double`, …) | `FLOAT` | decimal float |

**Custom specialization**

Provide `type_str`, `parse()`, and a `std::formatter<T>` (so defaults can be rendered in help text):

```cpp
enum class Color { Red, Green, Blue };

template <> struct std::formatter<Color> : std::formatter<std::string_view> {
    auto format(Color c, std::format_context &ctx) const {
        std::string_view name;
        switch (c) {
        case Color::Red:   name = "red";   break;
        case Color::Green: name = "green"; break;
        case Color::Blue:  name = "blue";  break;
        }
        return std::formatter<std::string_view>::format(name, ctx);
    }
};

template <> struct quikcli::ArgType<Color> {
    static constexpr std::string_view type_str = "COLOR";
    static Color parse(std::string_view sv) {
        if (sv == "red")   return Color::Red;
        if (sv == "green") return Color::Green;
        if (sv == "blue")  return Color::Blue;
        throw quikcli::ParseError(
            std::format("'{}' is not a valid color - expected red, green, or blue", sv));
    }
};
```

---

### `Flag<T>`

A `Flag<T>` represents one command-line parameter. Create flags via static factory methods, then optionally chain `.doc()` and `.alias()`. It is required that `T` is a valid `ArgType<T>`.

> [!NOTE]
> A `Flag<T>` is actually a `Flag<T, ExtractT=T>`. The `ExtractT` is the type after calling `extract()` on the flag. This is so that flags such as `optional` and `comma_delimited` can be extracted as an `optional<T>` and `vector<T>`. This subtlety can be largely ignored when composing flags in-place or using `auto`.

#### Named flags

Named flags appear on the command line as `--name VALUE`, `--name=VALUE`, or (with an alias) `-n VALUE` / `-nVALUE`.

| Factory | Extracted type | Required? | Description |
|---|---|---|---|
| `Flag<T>::required("name")` | `T` | yes | Must be supplied |
| `Flag<T>::optional("name")` | `std::optional<T>` | no | Absent -> `std::nullopt` |
| `Flag<T>::optional_with_default("name", val)` | `T` | no | Absent -> `val` |
| `Flag<bool>::no_arg("name")` | `bool` | no | Presence -> `true` |
| `Flag<T>::comma_delimited("name")` | `std::vector<T>` | no | `a,b,c` -> vector |

#### Positional (anonymous) flags

Positional arguments are matched by position, not by name. Their ordering rule is: required -> optional/default -> variadic.

| Factory | Extracted type | Description |
|---|---|---|
| `Flag<T>::anon("name")` | `T` | Required positional |
| `Flag<T>::anon_optional("name")` | `std::optional<T>` | Optional positional |
| `Flag<T>::anon_optional_with_default("name", val)` | `T` | Optional positional with default |
| `Flag<T>::anon_variadic("name")` | `std::vector<T>` | Collects all remaining positionals |

---

### `Param<T>`

A `Param<T>` groups one or more flags, where `T` is the extracted type.

#### Composing with `&`

Combining two params with `&` produces a new param whose extracted type is a flattened tuple of both:

```cpp
Param<std::tuple<std::string, int>> addr = Flag<std::string>::required("host") & Flag<int>::required("port");
```

Compositions are non-destructive: `addr` can be further composed without being consumed.

```cpp
auto with_verbose = addr & Flag<bool>::no_arg("verbose");
auto with_debug   = addr & Flag<bool>::no_arg("debug");
```

#### Mapping with `|`

The pipe operator applies a callable to the extracted tuple values and returns a new `Param` of the callable's return type:

```cpp
struct Server { std::string host; int port; };

Param<Server> server_param = Flag<std::string>::required("host") & Flag<int>::required("port")
    | [](std::string host, int port) { return Server{host, port}; };
```

Pipes can be chained: compose a mapped param with more flags, then pipe again:

```cpp
struct Config { Server server; bool verbose; };

auto config_param = server_param & Flag<bool>::no_arg("verbose")
    | [](Server s, bool v) { return Config{s, v}; };
```

---

### `Command`

#### `Command::basic` - leaf command

```cpp
Command::basic(std::string summary, Param<T> param, F callback)
```

where `callback` is called with the extracted value(s) of `param` when the command runs. If `T` is a tuple the callback receives each element as a separate argument; if `T` is a plain type (after `|`) it receives one argument. An example would be:

```cpp
auto cmd = Command::basic(
    "Greet the user.",
    Flag<std::string>::required("name").doc("who to greet"),
    [](std::string name) { std::cout << "Hello, " << name << "!\n"; });
```

Built-in flags `help` and `version` are injected automatically.

#### `Command::group` - subcommand router

```cpp
Command::group(std::string summary, std::vector<std::pair<std::string, Command>> subcommands)
```

Routes to the subcommand named by the first CLI argument. Groups can be nested to arbitrary depth:

```cpp
auto cloud = Command::group("Deploy to a cloud provider.", {
    {"aws", make_aws_cmd()},
    {"gcp", make_gcp_cmd()},
});

auto root = Command::group("Manage deployments.", {
    {"cloud",  std::move(cloud)},
    {"local",  make_local_cmd()},
});
```

Built-in subcommands `help` and `version` are injected automatically.

#### `Command::run`

```cpp
void run(int argc, char **argv, std::string_view version,
         std::ostream &out = std::cout, std::ostream &err = std::cerr);
```

Entry point. Pass `argc` and `argv` directly from `main` and a version string that is printed when `--version` or `-V` (for `basic` commands) or `version` (as a subcommand) is passed. Errors are written to `err`; help and version output go to `out`.

---

## Build & Test

QuikCli uses CMake. A `Makefile` wraps the common workflows:

```sh
# Configure and run all tests
make test

# Build the bundled examples
make examples

# Configure, build, and run everything
make all
```

---
