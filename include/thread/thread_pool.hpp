#pragma once
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <type_traits>

namespace atom::utils {

class thread_pool final {
public:
    thread_pool(const std::size_t num_threads = std::thread::hardware_concurrency())
        : num_threads_(num_threads) {
        try {
            for (auto i = 0; i < num_threads / 2; ++i) {
                emplace_thread();
            }
        }
        catch (...) {
            threads_.clear();
            num_threads_ = 0;
            throw;
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

        condvar_.notify_all();

        // jthread
    }

    /**
     * @brief Add a new task to the queue.
     *
     * @tparam Callable The type of the task, could be deduced.
     * @tparam Args Argument types, could be deduced.
     * @param callable The task.
     * @param args Arguments for finish the task.
     * @return Future.
     */
    template <typename Callable, typename... Args>
    auto enqueue(Callable&& callable, Args&&... args)
        -> std::future<std::invoke_result_t<Callable, Args...>> {
        using return_type = std::invoke_result_t<Callable, Args...>;

        if (stop_) [[unlikely]] {
            throw std::runtime_error("enqueue on stopped thread pool");
        }

        if (threads_.size() ^ num_threads_) {
            emplace_thread();
        }

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Callable>(callable), std::forward<Args>(args)...));

        std::future<return_type> future = task->get_future();

        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.emplace([task]() { (*task)(); });
        }
        condvar_.notify_one();

        return future;
    }

private:
    void emplace_thread() {
        threads_.emplace_back([this]() {
            while (true) {
                std::unique_lock<std::mutex> lock(mutex_);
                condvar_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });

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

    std::atomic<bool> stop_{ false };
    uint32_t num_threads_;
    std::mutex mutex_;
    std::vector<std::jthread> threads_;
    std::condition_variable condvar_;
    std::queue<std::function<void()>> tasks_;
};

} // namespace atom::utils
