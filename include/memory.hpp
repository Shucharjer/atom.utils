#pragma once
#include "concepts/mempool.hpp"

namespace atom::utils {

struct basic_allocator;

template <typename>
struct standard_allocator;

template <typename, ::atom::utils::concepts::mempool>
class allocator;

} // namespace atom::utils
