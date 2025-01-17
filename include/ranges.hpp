#pragma once
#include <ranges>

#define URANGES ::atom::utils::ranges::

namespace atom::utils::ranges {

template <std::ranges::input_range Rng>
class phony_input_iterator;

template <typename Rng, typename... Args>
class closure;

} // namespace atom::utils::ranges
