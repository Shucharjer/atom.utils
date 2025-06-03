#pragma once
#include <concepts>
#include <memory_resource>
#include "core.hpp"
#include "memory.hpp"

namespace atom::utils {

constexpr std::size_t k_default_page_size = 32;

template <
    std::unsigned_integral Ty, typename Alloc = std::allocator<Ty>,
    std::size_t = k_default_page_size>
class dense_set;

#if HAS_CXX20
template <
    std::unsigned_integral Kty, typename Ty, typename Alloc = std::allocator<std::pair<Kty, Ty>>,
    std::size_t = k_default_page_size>
#elif HAS_CXX17
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

} // namespace atom::utils
