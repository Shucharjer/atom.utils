#pragma once
#include <memory>
#include "concepts/mempool.hpp"

namespace atom::utils {

struct basic_allocator;

template <typename>
struct standard_allocator;

template <typename, ::atom::utils::concepts::mempool>
class allocator;

class basic_storage;

template <typename Ty, typename Alloc = std::allocator<Ty>>
class unique_storage;

template <typename Ty, typename Alloc = std::allocator<Ty>>
class shared_storage;

} // namespace atom::utils
