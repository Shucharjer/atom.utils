#pragma once
#include <concepts>
#include "concepts/allocator.hpp"
#include "memory.hpp"
#include "memory/allocator.hpp"

namespace atom::utils {

constexpr std::size_t k_default_page_size = 32;

template <
    std::unsigned_integral Ty,
    UCONCEPTS rebindable_allocator Alloc = UTILS standard_allocator<Ty>,
    std::size_t                          = k_default_page_size>
class dense_set;

template <
    std::unsigned_integral Key,
    typename Val,
    UCONCEPTS rebindable_allocator Alloc =
        UTILS standard_allocator<UTILS compressed_pair<Key, Val>>,
    std::size_t = k_default_page_size>
class dense_map;

} // namespace atom::utils
