#pragma once
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include "type_traits.h"

namespace atom::utils {

namespace internal {
constexpr std::size_t default_page_size = 32;
}

template <
    std::integral Key,
    typename,
    std::size_t PageSize             = internal::default_page_size,
    std::unsigned_integral IndexType = std::size_t>
requires is_positive_integral_v<PageSize>
class sparse_map;

template <
    std::integral Ty,
    std::size_t PageSize             = internal::default_page_size,
    std::unsigned_integral IndexType = std::size_t>
requires is_positive_integral_v<PageSize>
class sparse_set;
} // namespace atom::utils
