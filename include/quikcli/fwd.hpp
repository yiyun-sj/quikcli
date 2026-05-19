#pragma once
#include <stdexcept>
#include <string>

namespace quikcli {

template <typename T> struct ArgType;
template <typename T> class Flag;
template <typename T> class Param;
template <typename... Ts> class Params;
class Command;

struct ParseError : std::runtime_error {
    explicit ParseError(std::string msg) : std::runtime_error(std::move(msg)) {}
};

} // namespace quikcli