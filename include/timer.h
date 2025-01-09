#pragma once
#include <chrono>
#include <unordered_map>
#include "utils_macro.h"

namespace atom::utils {

class timer_proxy {
public:
    timer_proxy() noexcept : start_(std::chrono::high_resolution_clock::now()) {}
    timer_proxy(const timer_proxy&) noexcept = default;
    timer_proxy(timer_proxy&& that) noexcept : start_(that.start_) {}
    timer_proxy& operator=(const timer_proxy&) noexcept = default;
    timer_proxy& operator=(timer_proxy&& that) noexcept {
        if (this != &that) {
            start_ = that.start_;
        }
        return *this;
    }
    ~timer_proxy() = default;

    void from_now() noexcept { start_ = std::chrono::high_resolution_clock::now(); }

    [[nodiscard]] auto to_now() const noexcept -> std::chrono::duration<double> {
        const auto end = std::chrono::high_resolution_clock::now();
        return end - start_;
    }

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

class timer {
public:
    timer()                        = default;
    timer(const timer&)            = default;
    timer(timer&&)                 = default;
    timer& operator=(const timer&) = default;
    timer& operator=(timer&&)      = default;
    ~timer()                       = default;

    auto operator[](const char* timer_name) -> UTILS timer_proxy& {
        auto& timers_ = timers();

        if (!timers_.contains(timer_name)) {
            timers_.emplace(timer_name, UTILS timer_proxy());
        }

        return timers_[timer_name];
    }

    auto operator[](const char* timer_name) const -> const timer_proxy& {
        return timers()[timer_name];
    }

private:
    static auto timers() -> std::unordered_map<std::string, UTILS timer_proxy>& {
        static std::unordered_map<std::string, UTILS timer_proxy> timers_;
        return timers_;
    }
};

} // namespace atom::utils
