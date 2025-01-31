#pragma once
#include "core.hpp"
#include "memory/allocator.hpp"

namespace atom::utils {

template <typename>
class delegate;

class basic_sink;

template <
    typename EventType,
    typename = standard_allocator<std::pair<const default_id_t, delegate<void(EventType)>>>>
class sink;

template <typename = standard_allocator<std::pair<const default_id_t, basic_sink*>>>
class dispatcher;

} // namespace atom::utils
