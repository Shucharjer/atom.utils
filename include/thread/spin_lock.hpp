#pragma once
#include <atomic>
#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
    #include <xmmintrin.h>
#endif

namespace atom::utils {

namespace internal {

#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
inline void cpu_relax() { _mm_pause(); }
#elif defined(__aarch64__)
inline void cpu_relax() { __asm__ volatile("yield"); }
#else
inline void cpu_relax() {}
#endif

} // namespace internal

class spin_lock {
public:
    spin_lock()                            = default;
    spin_lock(const spin_lock&)            = delete;
    spin_lock& operator=(const spin_lock&) = delete;
    spin_lock(spin_lock&&)                 = delete;
    spin_lock& operator=(spin_lock&&)      = delete;
    ~spin_lock()                           = default;

    auto try_lock() noexcept -> bool { return !busy_.exchange(true, std::memory_order_acquire); }

    void lock() noexcept {
        while (!busy_.exchange(true, std::memory_order_acquire)) {
            while (busy_.load(std::memory_order_relaxed)) {
                internal::cpu_relax();
            }
        }
    }

    void unlock() noexcept { busy_.store(false, std::memory_order_release); }

private:
    std::atomic<bool> busy_{ false };
};

} // namespace atom::utils
