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

} // namespace internal
/*! @endcond */

/**
 * @class spin_lock
 * @brief Spin lock, suitable for high-frequency task scenarios.
 *
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
    auto try_lock() noexcept -> bool { return !busy_.exchange(true, std::memory_order_acquire); }

    void lock() noexcept {
        while (busy_.exchange(true, std::memory_order_acquire)) {
            while (busy_.load(std::memory_order_relaxed)) {
                internal::cpu_relax();
            }
        }
    }

    void unlock() noexcept { busy_.store(false, std::memory_order_release); }

private:
    std::atomic<bool> busy_{ false };
};

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
            if (!busy_.load(std::memory_order_relaxed)) {
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
                expected, true, std::memory_order_acquire, std::memory_order_relaxed
            );
        });
    }

    void unlock() noexcept {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            busy_.store(false, std::memory_order_release);
        }
        condvar_.notify_one();
    }

    bool try_lock() noexcept {
        bool expected = false;
        return busy_.compare_exchange_strong(
            expected, true, std::memory_order_acquire, std::memory_order_relaxed
        );
    }

private:
    std::atomic<bool> busy_{ false };
    std::mutex mutex_;
    std::condition_variable condvar_;
};

} // namespace atom::utils
