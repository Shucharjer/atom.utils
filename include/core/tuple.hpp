#pragma once
#include "concepts/type.hpp"

namespace atom::utils {

template <typename Ty, typename... Args, template <typename...> class Tuple>
requires ::atom::utils::concepts::completed<Ty> && ::atom::utils::concepts::tuple<Tuple<Args...>>
[[nodiscard]] constexpr Ty to(const Tuple<Args...>& tuple) {
    return [&tuple]<std::size_t... Is>(std::index_sequence<Is...>) {
        return Ty(std::get<Is>(tuple)...);
    }(std::make_index_sequence<sizeof...(Args)>());
}

// NOLINTBEGIN(cppcoreguidelines-rvalue-reference-param-not-moved)
template <typename Ty, typename... Args, template <typename...> class Tuple>
requires ::atom::utils::concepts::completed<Ty> && ::atom::utils::concepts::tuple<Tuple<Args...>>
[[nodiscard]] constexpr Ty to(Tuple<Args...>&& tuple) {
    // NOLINTEND(cppcoreguidelines-rvalue-reference-param-not-moved)
    return [&tuple]<std::size_t... Is>(std::index_sequence<Is...>) {
        return Ty(std::move(std::get<Is>(tuple))...);
    }(std::make_index_sequence<sizeof...(Args)>());
}

} // namespace atom::utils
