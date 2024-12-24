#pragma once
#include <memory>
#include "compressed_pair.h"
#include "signal.h"
#include "storage.h"

namespace atom::utils {

template <typename Ty>
class around_ptr {
public:
    using value_type    = Ty;
    using delegate_type = delegate<void(const Ty&)>;
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

} // namespace atom::utils
