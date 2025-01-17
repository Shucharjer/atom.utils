#pragma once
#include <algorithm>
#include <array>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <vector>

namespace atom::utils {

class spin_lock;

class thread_pool;

template <size_t, typename = std::shared_mutex, template <typename> typename = std::unique_lock>
class lock_keeper;

} // namespace atom::utils
