/*
 * Copyright (c) 2026 Yiyun Jia
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include "flag.hpp"

#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

namespace quikcli {

namespace detail {

template <typename T> struct is_tuple : std::false_type {};
template <typename... Ts> struct is_tuple<std::tuple<Ts...>> : std::true_type {};
template <typename T> inline constexpr bool is_tuple_v = is_tuple<T>::value;

template <typename T> using as_tuple_t = std::conditional_t<is_tuple_v<T>, T, std::tuple<T>>;

template <typename T, typename U>
using tuple_cat_t =
    decltype(std::tuple_cat(std::declval<as_tuple_t<T>>(), std::declval<as_tuple_t<U>>()));

template <typename T> auto as_tuple(T &&val) {
    if constexpr (is_tuple_v<std::decay_t<T>>)
        return std::forward<T>(val);
    else
        return std::make_tuple(std::forward<T>(val));
}

} // namespace detail

template <typename T> class Param {
  public:
    template <typename FlagT> Param(Flag<FlagT, T> &&flag) {
        auto shared = std::make_shared<Flag<FlagT, T>>(std::move(flag));
        specs_ = {&shared->spec()};
        extract_ = [f = std::move(shared)]() { return f->extract(); };
    }

    template <typename U> Param<detail::tuple_cat_t<T, U>> operator&(Param<U> rhs) const {
        std::vector<const FlagSpec *> merged = specs_;
        merged.insert(merged.end(), rhs.specs_.begin(), rhs.specs_.end());
        return Param<detail::tuple_cat_t<T, U>>(
            std::move(merged), [l = extract_, r = std::move(rhs.extract_)]() {
                return std::tuple_cat(detail::as_tuple(l()), detail::as_tuple(r()));
            });
    }

    template <typename FlagU, typename U>
    Param<detail::tuple_cat_t<T, U>> operator&(Flag<FlagU, U> &&rhs) const {
        return *this & Param<U>(std::move(rhs));
    }

    template <typename F> auto operator|(F &&f) const {
        using U = decltype(std::apply(std::declval<std::decay_t<F> &>(),
                                      std::declval<detail::as_tuple_t<T>>()));
        return Param<U>(specs_, [inner = extract_, f_ = std::forward<F>(f)]() {
            return std::apply(f_, detail::as_tuple(inner()));
        });
    }

    std::vector<const FlagSpec *> specs() const { return specs_; }
    T extract() const { return extract_(); }

  private:
    template <typename U> friend class Param;

    Param(std::vector<const FlagSpec *> specs, std::function<T()> extract)
        : specs_(std::move(specs)), extract_(std::move(extract)) {}

    std::vector<const FlagSpec *> specs_;
    std::function<T()> extract_;
};

template <typename T, typename ExtractT, typename U, typename ExtractU>
Param<std::tuple<ExtractT, ExtractU>> operator&(Flag<T, ExtractT> &&lhs, Flag<U, ExtractU> &&rhs) {
    return Param<ExtractT>(std::move(lhs)) & Param<ExtractU>(std::move(rhs));
}

template <typename T, typename ExtractT, typename ParamU>
Param<detail::tuple_cat_t<ExtractT, ParamU>> operator&(Flag<T, ExtractT> &&lhs, Param<ParamU> rhs) {
    return Param<ExtractT>(std::move(lhs)) & rhs;
}

} // namespace quikcli
