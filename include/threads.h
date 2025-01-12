#include <algorithm>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <vector>

namespace atom::utils {

class thread_pool {
public:
    thread_pool(const std::size_t num_threads) {
        for (auto i = 0; i < num_threads; ++i) {
            threads_.emplace_back([this]() {
                while (true) {
                    std::unique_lock<std::mutex> lock(mutex_);
                    condition_.wait(lock, [this]() { return !tasks_.empty(); });

                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    auto task{ std::move(tasks_.front()) };
                    tasks_.pop();
                    lock.unlock();
                    task();
                }
            });
        }
    }

    thread_pool(const thread_pool&) = delete;
    thread_pool(thread_pool&&)      = delete;

    thread_pool& operator=(const thread_pool&) = delete;
    thread_pool& operator=(thread_pool&&)      = delete;

    ~thread_pool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }

        condition_.notify_all();
        auto join = [](auto& thread) { thread.join(); };
        std::ranges::for_each(threads_, join);
    }

    template <typename Callable, typename... Args>
    void new_task(Callable&& callable, Args&&... args) {
        std::function<void()> task =
            std::bind(std::forward<Callable>(callable), std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.emplace(std::move(task));
        }
        condition_.notify_one();
    }

    [[nodiscard]] bool no_task() const noexcept {
        // TODO:
        return true;
    }

private:
    bool stop_ = false;
    std::vector<std::thread> threads_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<std::function<void()>> tasks_;
};

template <size_t, typename = std::shared_mutex, template <typename> typename = std::unique_lock>
class lock_keeper;

template <size_t Count, typename Mutex, template <typename> typename Lock>
class lock_keeper {
public:
    template <typename... Mutexes>
    explicit lock_keeper(Mutexes&... mutexes) noexcept : locks_{ Lock(mutexes)... } {}

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
