#pragma once
#include <set>
#include <unordered_set>
#include "memory.hpp"
#include "memory/allocator.hpp"
#include "memory/pool.hpp"
#include "structures.hpp"

namespace atom::utils {

template <typename Ty, typename Pr = std::less<Ty>>
using sync_set = ::std::set<Ty, Pr, sync_allocator<const Ty>>;

template <typename Ty, typename Hasher = std::hash<Ty>, typename Keyeq = std::equal_to<Ty>>
using sync_unordered_set = ::std::unordered_set<Ty, Hasher, Keyeq, sync_allocator<const Ty>>;

template <typename Ty, typename Pr = std::less<Ty>>
using unsync_set = ::std::set<Ty, Pr, unsync_allocator<const Ty>>;

template <typename Ty, typename Hasher = std::hash<Ty>, typename Keyeq = std::equal_to<Ty>>
using unsync_unordered_set = ::std::unordered_set<Ty, Hasher, Keyeq, unsync_allocator<const Ty>>;

} // namespace atom::utils
