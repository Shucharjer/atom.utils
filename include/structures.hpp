#pragma once
#include <concepts>
#include "concepts/allocator.hpp"
#include "core/pair.hpp"
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

#if _HAS_CXX20
template <
    std::unsigned_integral Kty, typename Ty, typename Alloc = std::allocator<std::pair<Kty, Ty>>,
    std::size_t = k_default_page_size>
#elif _HAS_CXX17
template <
    typename Kty, typename Ty, typename Alloc, std::size_t PageSize,
    typename = std::enable_if_t<std::is_integral_v<Kty> && std::is_unsigned_v<Kty>>>
#endif
class dense_map;

#if __has_include(<memory_resource>)
namespace pmr {

template <std::unsigned_integral Key, typename Val, std::size_t size = k_default_page_size>
using dense_map = dense_map<Key, Val, std::pmr::polymorphic_allocator<std::pair<Key, Val>>, size>;
}

#endif

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
