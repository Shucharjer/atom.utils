#pragma once
#include <ranges>

#define URANGES ::atom::utils::ranges::

namespace atom::utils {

namespace ranges {
template <std::ranges::input_range Rng>
class phony_input_iterator;

template <typename Container, std::ranges::input_range Rng, typename... Args>
requires(!std::ranges::view<Container>)
[[nodiscard]] constexpr Container to(Rng&& range, Args&&... args);

template <typename Container, typename... Args>
[[nodiscard]] constexpr auto to(Args&&... args);

template <template <typename...> typename Container, typename... Args>
[[nodiscard]] constexpr auto to(Args&&... args);

template <std::ranges::range Rng, size_t Index, bool IsConst>
struct element_iterator;

template <std::ranges::input_range Vw, size_t Index>
class elements_view;

template <size_t Index>
struct element_fn;

namespace views {}

} // namespace ranges

namespace views = ranges::views;

} // namespace atom::utils
