#pragma once
#include <forward_list>
#include <list>
#include <queue>
#include <stack>
#include <vector>
#include "structures.hpp"

namespace atom::utils {

template <typename Ty>
using sync_vector = std::vector<Ty, sync_allocator<Ty>>;

template <typename Ty>
using sync_list = std::list<Ty, sync_allocator<Ty>>;

template <typename Ty>
using sync_forward_list = std::forward_list<Ty, sync_allocator<Ty>>;

template <typename Ty>
using sync_deque = std::deque<Ty, sync_allocator<Ty>>;

template <typename Ty>
using sync_queue = std::queue<Ty, sync_deque<Ty>>;

template <
    typename Ty,
    typename Container = sync_vector<Ty>,
    typename Pr        = std::less<typename Container::value_type>>
using sync_priority_queue = std::priority_queue<Ty, Container, Pr>;

template <typename Ty, typename Container = sync_deque<Ty>>
using sync_stack = std::stack<Ty, Container>;

template <typename Ty>
using unsync_vector = std::vector<Ty, unsync_allocator<Ty>>;

template <typename Ty>
using unsync_list = std::list<Ty, unsync_allocator<Ty>>;

template <typename Ty>
using unsync_forward_list = std::forward_list<Ty, unsync_allocator<Ty>>;

template <typename Ty>
using unsync_deque = std::deque<Ty, unsync_allocator<Ty>>;

template <typename Ty, typename Container = unsync_deque<Ty>>
using unsync_queue = std::queue<Ty, Container>;

template <
    typename Ty,
    typename Container = unsync_vector<Ty>,
    typename Pr        = std::less<typename Container::value_type>>
using unsync_priority_queue = std::priority_queue<Ty, Container, Pr>;

template <typename Ty, typename Container = unsync_deque<Ty>>
using unsync_stack = std::stack<Ty, Container>;

} // namespace atom::utils
