#pragma once
#include <ranges>

#define URANGES ::atom::utils::ranges::

namespace atom::utils::ranges {

template <std::ranges::input_range Rng>
class phony_input_iterator;

} // namespace atom::utils::ranges
