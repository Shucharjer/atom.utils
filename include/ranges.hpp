#pragma once
#include <ranges>

#define URANGES ::atom::utils::ranges::

namespace atom::utils {

namespace ranges {
template <std::ranges::input_range Rng>
class phony_input_iterator;
}

template <typename Container, std::ranges::input_range Rng, typename... Args>
requires(!std::ranges::view<Container>)
[[nodiscard]] constexpr Container to(Rng&& range, Args&&... args);

template <typename Container, typename... Args>
[[nodiscard]] constexpr auto to(Args&&... args);

template <template <typename...> typename Container, typename... Args>
[[nodiscard]] constexpr auto to(Args&&... args);

} // namespace atom::utils
