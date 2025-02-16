#pragma once
#include "thread.hpp"

namespace atom::utils {

template <size_t Count, typename Mutex, template <typename> typename Lock>
class lock_keeper {
public:
    template <typename... Mutexes>
    requires(sizeof...(Mutexes) == Count)
    explicit lock_keeper(Mutexes&... mutexes) noexcept
        : locks_{ Lock(const_cast<Mutex&>(mutexes))... } {}

    lock_keeper(const lock_keeper&)            = delete;
    lock_keeper(lock_keeper&&)                 = delete;
    lock_keeper& operator=(const lock_keeper&) = delete;
    lock_keeper& operator=(lock_keeper&&)      = delete;

    ~lock_keeper() noexcept = default;

    void run_away() noexcept {
        std::ranges::for_each(locks_, [](Lock<Mutex>& lock) { lock.unlock(); });
    }

private:
    std::array<Lock<Mutex>, Count> locks_;
};

} // namespace atom::utils
