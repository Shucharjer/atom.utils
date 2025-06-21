#pragma once

#include <type_traits>
#include "core.hpp"
#include "signal.hpp"
namespace atom::utils {

template <typename>
class around;
template <typename Ty>
class around_proxy;

template <typename Ty>
class around : private around_proxy<Ty> {
    friend class around_proxy<Ty>;
    Ty value_;

public:
    using proxy_type = around_proxy<Ty>;

    constexpr around() noexcept(std::is_nothrow_default_constructible_v<Ty>)
    requires std::is_default_constructible_v<Ty>
    = default;

    template <typename... Args>
    constexpr around(Args&&... args) noexcept(std::is_nothrow_constructible_v<Ty, Args...>)
    requires std::is_constructible_v<Ty, Args...>
        : value_(std::forward<Args>(args)...) {}

    template <typename T>
    constexpr around(const around<T>& that) noexcept(std::is_nothrow_constructible_v<Ty, T>)
    requires std::is_constructible_v<Ty, T>
        : value_(that.value) {}

    template <typename T>
    constexpr around(around<T>&& that) noexcept(std::is_nothrow_constructible_v<Ty, T>)
    requires std::is_constructible_v<Ty, T>
        : value_(std::move(that).value_) {}

    template <typename T>
    constexpr around& operator=(const around<T>& that) noexcept(std::is_nothrow_assignable_v<Ty, T>)
    requires std::is_assignable_v<Ty, T>
    {
        value_ = that.value_;
    }

    template <typename T>
    constexpr around& operator=(around<T>&& that) noexcept(std::is_nothrow_assignable_v<Ty, T>)
    requires std::is_assignable_v<Ty, T>
    {
        value_ = std::move(that).value_;
    }

    constexpr ~around() noexcept(std::is_nothrow_destructible_v<Ty>) = default;

    constexpr around_proxy<Ty>& operator->() noexcept { return *this; }
    constexpr const around_proxy<Ty>& operator->() const noexcept { return *this; }
};

template <typename Ty>
class around_proxy {
    using around_type = around<Ty>;

    delegate<void(Ty&)> delegate_;
    delegate<void(const Ty&)> cdelegate_;

    friend class around<Ty>;

    template <auto Candidata>
    constexpr around_proxy(spreader<Candidata> spreader) noexcept : delegate_(spreader) {}
    template <auto Candidata, typename T>
    constexpr around_proxy(spreader<Candidata> spreader, T& instance) noexcept
        : delegate_(spreader, instance) {}

public:
    around_proxy(const around_proxy&)            = delete;
    around_proxy(around_proxy&&)                 = delete;
    around_proxy& operator=(const around_proxy&) = delete;
    around_proxy& operator=(around_proxy&&)      = delete;

    ~around_proxy() noexcept = default;

    constexpr Ty* operator->() noexcept { return static_cast<around_type*>(this)->value_; }
    constexpr const Ty* operator->() const noexcept {
        return static_cast<const around_type*>(this)->value_;
    }
};

} // namespace atom::utils
