#pragma once
#include <concepts>
#include "concepts/allocator.hpp"
#include "memory.hpp"
#include "memory/allocator.hpp"
#include "memory/pool.hpp"

namespace atom::utils {

constexpr std::size_t k_default_page_size = 32;

template <
    std::unsigned_integral Ty,
    ::atom::utils::concepts::rebindable_allocator Alloc = ::atom::utils::standard_allocator<Ty>,
    std::size_t                                         = k_default_page_size>
class dense_set;

template <
    std::unsigned_integral Key,
    typename Val,
    ::atom::utils::concepts::rebindable_allocator Alloc =
        ::atom::utils::standard_allocator<::atom::utils::compressed_pair<Key, Val>>,
    std::size_t = k_default_page_size>
class dense_map;

template <typename Ty>
using sync_allocator = allocator<Ty, synchronized_pool>;

template <typename First, typename Second>
using sync_comperessed_allocator = sync_allocator<compressed_pair<First, Second>>;

template <typename First, typename Second>
using sync_pair_allocator = sync_allocator<std::pair<const First, Second>>;

template <typename Ty>
using unsync_allocator = allocator<Ty, unsynchronized_pool>;

template <typename First, typename Second>
using unsync_compressed_allocator = unsync_allocator<compressed_pair<First, Second>>;

template <typename First, typename Second>
using unsync_pair_allocator = sync_allocator<std::pair<const First, Second>>;

} // namespace atom::utils
