#pragma once
#include "concepts/allocator.hpp"
#include "core.hpp"
#include "memory/allocator.hpp"

namespace atom::utils {

template <typename>
class delegate;

class basic_sink;

template <
    typename EventType,
    typename = standard_allocator<std::pair<const default_id_t, delegate<void(EventType&)>>>>
class sink;

template <
    concepts::rebindable_allocator = standard_allocator<std::pair<const default_id_t, basic_sink*>>>
class dispatcher;

template <typename Alloc = std::allocator<compressed_pair<default_id_t, void*>>>
class transient_collection;

} // namespace atom::utils
