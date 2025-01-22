#pragma once
#include <map>
#include <unordered_map>
#include "memory.hpp"
#include "memory/allocator.hpp"
#include "memory/pool.hpp"
#include "structures.hpp"

namespace atom::utils {

template <typename Key, typename Val, typename Pr = std::less<Key>>
using sync_map = std::map<Key, Val, Pr, sync_allocator<std::pair<const Key, Val>>>;

template <
    typename Key,
    typename Val,
    typename Hasher = std::hash<Key>,
    typename Keyeq  = std::equal_to<Key>>
using sync_unordered_map =
    ::std::unordered_map<Key, Val, Hasher, Keyeq, sync_allocator<std::pair<const Key, Val>>>;

template <typename Key, typename Val, typename Pr = std::less<Key>>
using unsync_map = std::map<Key, Val, Pr, unsync_allocator<std::pair<const Key, Val>>>;

template <
    typename Key,
    typename Val,
    typename Hasher = std::hash<Key>,
    typename Keyeq  = std::equal_to<Key>>
using unsync_unordered_map =
    ::std::unordered_map<Key, Val, Hasher, Keyeq, unsync_allocator<std::pair<const Key, Val>>>;

} // namespace atom::utils
