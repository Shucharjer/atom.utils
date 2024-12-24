#pragma once
#include <atomic>
#include <utility>

namespace atom::utils {

template <typename Ty>
class global_counter {
public:
    global_counter() { ++count_; }
    global_counter(const global_counter& obj) { ++count_; }
    global_counter(global_counter&& obj) noexcept {}
    global_counter& operator=(const global_counter& obj) noexcept = default;
    global_counter& operator=(global_counter&& obj) noexcept { return *this; }
    ~global_counter() { --count_; }

    [[nodiscard]] auto count() const -> std::size_t { return count_; }

private:
    static std::atomic<std::size_t> count_;
};

class counter {
public:
    using meta_count_type = uint32_t;
    using count_type      = std::atomic<meta_count_type>;

    counter() noexcept = default;

    counter(const counter& that) noexcept : count_(that.count_) { inc(); }

    counter(counter&& that) noexcept : count_(std::exchange(that.count_, nullptr)) {}

    counter& operator=(const counter& that) {
        if (this != &that) {
            dec();
            count_ = that.count_;
            if (static_cast<bool>(count_)) {
                inc();
            }
        }

        return *this;
    }

    counter& operator=(counter&& that) noexcept {
        if (this != &that) {
            dec();
            count_ = std::exchange(that.count_, nullptr);
        }

        return *this;
    }

    ~counter() { dec(); }

    void inc() {
        if (!static_cast<bool>(count_)) {
            count_ = ::new count_type(1);
        }
        else {
            count_->fetch_add(1, std::memory_order_relaxed);
        }
    }

    void dec() {
        if (static_cast<bool>(count_)) {
            meta_count_type expected = 1;
            if (count_->load(std::memory_order_acquire) == 1 &&
                count_->compare_exchange_strong(expected, 0, std::memory_order_acq_rel)) {
                delete count_;
                count_ = nullptr;
            }
            else {
                count_->fetch_sub(1, std::memory_order_release);
            }
        }
    }

    [[nodiscard]] auto count() const noexcept -> size_t {
        return count_->load(std::memory_order_relaxed);
    }

private:
    count_type* count_{};
};

} // namespace atom::utils
