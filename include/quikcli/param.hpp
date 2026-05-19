#pragma once
#include "flag.hpp"

#include <tuple>
#include <vector>

namespace quikcli {

template <typename... Ts> class Param {
  public:
    explicit Param(Flag<Ts>... fs) : flags(std::move(fs)...) {}

    template <typename U> Param<Ts..., U> operator&(Flag<U> rhs) {
        return std::apply(
            [&rhs](auto &&...fs) {
                return Param<Ts..., U>(std::forward<decltype(fs)>(fs)..., std::move(rhs));
            },
            flags);
    }

    template <typename... Us> Param<Ts..., Us...> operator&(Param<Us...> rhs) {
        return std::apply(
            [&rhs](auto &&...ls) {
                return std::apply(
                    [&ls...](auto &&...rs) {
                        return Param<Ts..., Us...>(std::forward<decltype(ls)>(ls)...,
                                                   std::forward<decltype(rs)>(rs)...);
                    },
                    rhs.flags);
            },
            flags);
    }

    std::vector<const FlagSpec *> specs() {
        std::vector<const FlagSpec *> result;
        result.reserve(sizeof...(Ts));
        std::apply([&result](auto &...fs) { (result.push_back(&fs.spec()), ...); }, flags);
        return result;
    }

    std::tuple<Ts...> extract() {
        return std::apply([](auto &...fs) { return std::make_tuple(fs.extract()...); }, flags);
    }

  private:
    template <typename... Us> friend class Param;

    std::tuple<Flag<Ts>...> flags;
};

template <typename T, typename U> Param<T, U> operator&(Flag<T> lhs, Flag<U> rhs) {
    return Param<T, U>(std::move(lhs), std::move(rhs));
}

template <typename... Ts> Param<Ts...> param(Flag<Ts>... fs) { return Param<Ts...>(fs...); }

} // namespace quikcli
