#pragma once
#include "concepts/completed.hpp"
#include "concepts/tuple.hpp"

namespace atom::utils {

template <typename Ty, typename... Args, template <typename...> class Tuple>
// clang-format off
requires UCONCEPTS completed<Ty> && UCONCEPTS tuple<Tuple<Args...>>
[[nodiscard]] constexpr Ty to(const Tuple<Args...>& tuple) {
    // clang-format on
    return [&tuple]<std::size_t... Is>(std::index_sequence<Is...>) {
        return Ty(std::get<Is>(tuple)...);
    }(std::make_index_sequence<sizeof...(Args)>());
}

template <typename Ty, typename... Args, template <typename...> class Tuple>
// clang-format off
requires UCONCEPTS completed<Ty>&& UCONCEPTS tuple<Tuple<Args...>>
[[nodiscard]] constexpr Ty to(Tuple<Args...>&& tuple) {
    // clang-format on
    return [&tuple]<std::size_t... Is>(std::index_sequence<Is...>) {
        return Ty(std::move(std::get<Is>(tuple))...);
    }(std::make_index_sequence<sizeof...(Args)>());
}

} // namespace atom::utils
