#pragma once
#include <atomic>
#include <mutex>

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
    #include <xmmintrin.h>
#endif

namespace atom::utils {

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
inline void cpu_relax() { _mm_pause(); }
#elif defined(__aarch64__)
inline void cpu_relax() { __asm__ volatile("yield"); }
#else
inline void cpu_relax() {}
#endif

const auto max_spin_time = 1024;

} // namespace internal
/*! @endcond */

#if defined(__cpp_lib_atomic_wait) && __cpp_lib_atomic_wait >= 201907L

/**
 * @class spin_lock
 * @brief Spin lock, suitable for high-frequency task scenarios.
 * @details This implementation will detech notifications from other threads instead of simply spin.
 */
class spin_lock {
public:
    spin_lock()                            = default;
    spin_lock(const spin_lock&)            = delete;
    spin_lock& operator=(const spin_lock&) = delete;
    spin_lock(spin_lock&&)                 = delete;
    spin_lock& operator=(spin_lock&&)      = delete;
    ~spin_lock()                           = default;

    /**
     * @brief Try get the lock.
     *
     */
    auto try_lock() noexcept -> bool { return flag_.test_and_set(std::memory_order_acquire); }

    void lock() noexcept {
        while (flag_.test_and_set(std::memory_order_acquire)) {
    #if defined(__cpp_lib_atomic_wait) && __cpp_lib_atomic_wait >= 201907L
            flag_.wait(true, std::memory_order_relaxed);
    #endif
        }
    }

    void unlock() noexcept {
        flag_.clear(std::memory_order_release);
        flag_.notify_one();
    }

private:
    #if defined(__cpp_lib_atomic_value_initialization) &&                                          \
        __cpp_lib_atomic_value_initialization >= 201911L
    std::atomic_flag flag_{};
    #else
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
    #endif
};

#endif

/**
 * @class triditional_spin_lock
 * @brief Triditional spin lock, suitable for high-frequency task scenarios.
<<<<<<< HEAD
 * @details Normal spin lock, spin when calling `lock()`, which means low latency.
=======
 * @details Normal spin lock, spin when in `lock()`, which means low latency.
>>>>>>> 3912dbef686cf8b93f99e60e4dda248681331e64
 */
class triditional_spin_lock {
public:
    triditional_spin_lock()                                        = default;
    triditional_spin_lock(const triditional_spin_lock&)            = delete;
    triditional_spin_lock(triditional_spin_lock&&)                 = delete;
    triditional_spin_lock& operator=(const triditional_spin_lock&) = delete;
    triditional_spin_lock& operator=(triditional_spin_lock&&)      = delete;
    ~triditional_spin_lock()                                       = default;

    /**
     * @brief Try get the lock.
     *
     */
    auto try_lock() noexcept -> bool { return flag_.test_and_set(std::memory_order_acquire); }

    void lock() noexcept {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            internal::cpu_relax();
        }
    }

    void unlock() noexcept { flag_.clear(std::memory_order_release); }

private:
#if defined(__cpp_lib_atomic_value_initialization) &&                                              \
    __cpp_lib_atomic_value_initialization >= 201911L
    std::atomic_flag flag_{};
#else
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
#endif
};

#if defined(__cpp_lib_atomic_wait) && __cpp_lib_atomic_wait >= 201907L

/**
 * @class hybrid_spin_lock
 * @brief A new implementation of spin lock inspired by atomic_wiat in c++20
 * @details It will first try triditional spinning and wait for notification if it cannot lock for a
 * long time, which means versatility.
 */
class hybrid_spin_lock {
public:
    hybrid_spin_lock()                                   = default;
    hybrid_spin_lock(const hybrid_spin_lock&)            = delete;
    hybrid_spin_lock(hybrid_spin_lock&&)                 = delete;
    hybrid_spin_lock& operator=(const hybrid_spin_lock&) = delete;
    hybrid_spin_lock& operator=(hybrid_spin_lock&&)      = delete;
    ~hybrid_spin_lock()                                  = default;

    auto try_lock() noexcept -> bool { return flag_.test_and_set(std::memory_order_acquire); }

    void lock() noexcept {
        for (auto i = 0;
             i < internal::max_spin_time && flag_.test_and_set(std::memory_order_acquire); ++i) {
            internal::cpu_relax();
        }

        while (flag_.test_and_set(std::memory_order_acquire)) {
            flag_.wait(true, std::memory_order_relaxed);
        }
    }

<<<<<<< HEAD
    void unlock() noexcept {
        flag_.clear(std::memory_order_release);
        flag_.notify_one();
    }
=======
    void unlock() noexcept { flag_.clear(std::memory_order_release); }
>>>>>>> 3912dbef686cf8b93f99e60e4dda248681331e64

private:
    #if defined(__cpp_lib_atomic_value_initialization) &&                                          \
        __cpp_lib_atomic_value_initialization >= 201911L
    std::atomic_flag flag_{};
    #else
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
    #endif
};

#else

using spin_lock = triditional_spin_lock;

#endif

/**
 * @class hybrid_lock
 * @brief Hybrid lock, first try self-selection, then degenerate to mutual exclusion lock.
 *
 */
class hybrid_lock {
public:
    hybrid_lock()                              = default;
    hybrid_lock(const hybrid_lock&)            = delete;
    hybrid_lock(hybrid_lock&&)                 = delete;
    hybrid_lock& operator=(const hybrid_lock&) = delete;
    hybrid_lock& operator=(hybrid_lock&&)      = delete;
    ~hybrid_lock()                             = default;

    void lock() noexcept {
        // spin
        const int spin_limit  = 1024;
        const int max_backoff = 10;
        const int num         = 10;
        int spin_count        = 0;
        while (spin_count++ < spin_limit) {
            // pre-check
            if (!busy_.load(std::memory_order_acquire)) {
                if (try_lock()) {
                    return;
                }
            }

            // exponential backoff
            int backoff = std::min(max_backoff, spin_count / num);
            for (int i = 0; i < (1 << backoff); ++i) {
                internal::cpu_relax();
            }
        }

        // mutex
        std::unique_lock<std::mutex> lock(mutex_);
        condvar_.wait(lock, [this] {
            bool expected = false;
            return !busy_.compare_exchange_strong(
                expected, true, std::memory_order_acquire, std::memory_order_relaxed);
        });
    }

    void unlock() noexcept {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            busy_.store(false, std::memory_order_relaxed);
        }
        condvar_.notify_one();
    }

    bool try_lock() noexcept {
        bool expected = false;
        return busy_.compare_exchange_strong(
            expected, true, std::memory_order_acquire, std::memory_order_acquire);
    }

private:
    std::atomic<bool> busy_{ false };
    std::mutex mutex_;
    std::condition_variable condvar_;
};

} // namespace atom::utils
