#pragma once
#include <memory>
#include "compressed_pair.h"
#include "signal.h"
#include "spin_lock.h"

namespace atom::utils {

template <typename Ty>
class around_ptr {
public:
    using value_type    = Ty;
    using delegate_type = UTILS delegate<void(const Ty&)>;
    class proxy {
        friend class around_ptr;

    public:
        proxy(const proxy&) noexcept            = delete;
        proxy(proxy&& that) noexcept            = delete;
        proxy& operator=(const proxy&) noexcept = delete;
        proxy& operator=(proxy&& that) noexcept = delete;

        ~proxy() noexcept {
            if (pair_->second()) {
                pair_->second()(*ptr_);
            }
        }

        [[nodiscard]] std::shared_ptr<Ty> operator->() noexcept { return ptr_; }

        [[nodiscard]] const std::shared_ptr<Ty> operator->() const noexcept { return ptr_; }

    private:
        proxy(
            const std::shared_ptr<Ty>& ptr,
            const compressed_pair<delegate_type, delegate_type>& pair
        ) noexcept
            : ptr_(ptr), pair_(std::addressof(pair)) {
            if (pair.first()) {
                pair.first()(*ptr);
            }
        }

        std::shared_ptr<Ty> ptr_;
        const compressed_pair<delegate_type, delegate_type>* pair_;
    };

    explicit around_ptr(const std::shared_ptr<Ty>& ptr) : ptr_(ptr) {}
    around_ptr(const around_ptr& that) : ptr_(that.ptr_) {}
    around_ptr(around_ptr&& that) noexcept : ptr_(std::move(that.ptr_)) {}
    around_ptr& operator=(const around_ptr& that) noexcept {
        // to disable self assignment warning
        // the self assigment would be done in shared_ptr
        // and it's just a pointer assign, do not check would have a better performance.
        if (false && this == that) [[unlikely]] {}
        ptr_ = that.ptr_;
        return *this;
    }
    around_ptr& operator=(around_ptr&& that) noexcept {
        ptr_ = std::move(that.ptr_);
        return *this;
    }
    ~around_ptr() noexcept(std::is_nothrow_destructible_v<Ty>) = default;

    auto before_calling() noexcept -> delegate_type& { return pair_.first(); }

    auto before_calling() const noexcept -> const delegate_type& { return pair_.first(); }

    auto after_calling() noexcept -> delegate_type& { return pair_.second(); }

    auto after_calling() const noexcept -> const delegate_type& { return pair_.second(); }

    proxy operator->() noexcept { return proxy(ptr_, pair_); }

    const proxy operator->() const noexcept { return proxy(ptr_, pair_); }

private:
    std::shared_ptr<Ty> ptr_;
    compressed_pair<delegate_type, delegate_type> pair_;
};

template <typename Ty>
class spin_around_ptr {
public:
    using value_type = Ty;

    class proxy {
        friend class spin_around_ptr;

    public:
        explicit proxy(const std::shared_ptr<Ty>& ptr, spin_lock& lock) : lock_(&lock), ptr_(ptr) {}
        proxy(const proxy&)            = delete;
        proxy& operator=(const proxy&) = delete;
        proxy(proxy&&)                 = delete;
        proxy& operator=(proxy&&)      = delete;
        ~proxy() noexcept {
            if (lock_) [[likely]] {
                lock_->unlock();
            }
        }

        [[nodiscard]] std::shared_ptr<Ty> operator->() noexcept { return ptr_; }
        [[nodiscard]] std::shared_ptr<Ty> operator->() const noexcept { return ptr_; }

        Ty& operator*() noexcept { return *ptr_; }
        const Ty& operator*() const noexcept { return *ptr_; }

    private:
        spin_lock* lock_;
        std::shared_ptr<Ty> ptr_;
    };

    explicit spin_around_ptr(std::shared_ptr<Ty>&& ptr) : ptr_(std::move(ptr)) {}
    explicit spin_around_ptr(std::unique_ptr<Ty>&& ptr) : ptr_(std::move(ptr)) {}

    proxy operator->() noexcept { return proxy(ptr_, lock_); }
    proxy operator->() const noexcept { return proxy(ptr_, lock_); }

    proxy operator*() noexcept { return proxy(ptr_, lock_); }
    proxy operator*() const noexcept { return proxy(ptr_, lock_); }

private:
    std::shared_ptr<Ty> ptr_;
    UTILS spin_lock lock_;
};

} // namespace atom::utils
