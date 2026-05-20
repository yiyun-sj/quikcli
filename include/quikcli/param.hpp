#pragma once
#include "flag.hpp"

#include <tuple>
#include <vector>

namespace quikcli {

namespace detail {

template <typename F> struct flag_extract_type;
template <typename T, typename ExtractT> struct flag_extract_type<Flag<T, ExtractT>> {
    using type = ExtractT;
};
template <typename F> using flag_extract_type_t = typename flag_extract_type<F>::type;

} // namespace detail

template <typename... Ts> class Param {
  public:
    explicit Param(Ts... fs) : flags_(std::move(fs)...) {}

    template <typename U> Param<Ts..., U> operator&(U rhs) {
        return std::apply(
            [&rhs](auto &&...fs) {
                return Param<Ts..., U>(std::forward<decltype(fs)>(fs)..., std::move(rhs));
            },
            flags_);
    }

    template <typename... Us> Param<Ts..., Us...> operator&(Param<Us...> rhs) {
        return std::apply(
            [&rhs](auto &&...ls) {
                return std::apply(
                    [&ls...](auto &&...rs) {
                        return Param<Ts..., Us...>(std::forward<decltype(ls)>(ls)...,
                                                   std::forward<decltype(rs)>(rs)...);
                    },
                    rhs.flags_);
            },
            flags_);
    }

    std::vector<const FlagSpec *> specs() {
        std::vector<const FlagSpec *> result;
        result.reserve(sizeof...(Ts));
        std::apply([&result](auto &...fs) { (result.push_back(&fs.spec()), ...); }, flags_);
        return result;
    }

    std::tuple<detail::flag_extract_type_t<Ts>...> extract() {
        return std::apply([](auto &...fs) { return std::make_tuple(fs.extract()...); }, flags_);
    }

  private:
    template <typename... Us> friend class Param;

    std::tuple<Ts...> flags_;
};

template <typename T, typename ExtractT, typename U, typename ExtractU>
Param<Flag<T, ExtractT>, Flag<U, ExtractU>> operator&(Flag<T, ExtractT> lhs,
                                                      Flag<U, ExtractU> rhs) {
    return Param<Flag<T, ExtractT>, Flag<U, ExtractU>>(std::move(lhs), std::move(rhs));
}

template <typename... Ts> Param<Ts...> param(Ts... fs) { return Param<Ts...>(fs...); }

} // namespace quikcli
