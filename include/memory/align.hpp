#pragma once
#include <array>
#include <cstddef>
#include <new>
#include <type_traits>

#if __has_include(<memory/copy.hpp>)
    #include "memory/copy.hpp"
#else
    #if __has_include(<core/langdef.hpp>)
        #include "core/langdef.hpp"
    #else
constexpr auto num_sixteen = 16;
    #endif

namespace atom::utils {

/**
 * @brief Memcpy on runtime, used to assign to trivially assignable type.
 *
 * @param[out] dst Destination.
 * @param[in] src Source.
 */
template <typename Ty, typename T = Ty>
requires std::is_trivially_assignable_v<Ty, T>
constexpr void rtmemcpy(Ty& dst, T&& src) noexcept {
    if (std::is_constant_evaluated()) [[unlikely]] {
        dst = std::forward<T>(src);
    }
    else [[likely]] {
        if constexpr (sizeof(Ty) > num_sixteen) {
            std::memcpy(std::addressof(dst), std::addressof(src), sizeof(Ty));
        }
        else {
            dst = std::forward<T>(src);
        }
    }
}

} // namespace atom::utils

#endif

namespace atom::utils {

/**
 * @brief Structure contains aligned value with padding.
 *
 * @note N should be power of 2.
 * @tparam Ty Value's type.
 * @tparam N Alignment. Usually set this equal to or greater than
 * std::hardware_destructive_interference_size, but equal to or little than
 * std::hardware_constructive_interference_size.
 */
template <typename Ty, std::size_t N = std::hardware_destructive_interference_size>
struct alignas(N) aligned {
    Ty value;
    /**
     * @brief Padding
     * For most time, this is not useful.
     */
    std::array<std::byte, N - sizeof(Ty)> _padding;

    constexpr aligned() = default;

    template <typename T>
    requires std::is_constructible_v<Ty, T>
    constexpr aligned(T&& value) : value(std::forward<T>(value)), _padding() {}

    constexpr aligned(const aligned& that) noexcept(std::is_nothrow_copy_constructible_v<Ty>)
        : value(that.value), _padding() {}
    constexpr aligned(aligned&& that) noexcept(std::is_nothrow_move_constructible_v<Ty>)
        : value(std::move(that.value)), _padding() {}
    constexpr aligned& operator=(const aligned& that) noexcept {
        if (this != &that) [[likely]] {
            if constexpr (std::is_trivially_copy_assignable_v<Ty>) {
                rtmemcpy(value, that.value);
            }
            else {
                aligned temp(that);
                std::swap(value, temp.value);
            }
        }
        return *this;
    }
    constexpr aligned& operator=(aligned&& that) noexcept {
        if (this != &that) [[likely]] {
            value = std::move(that);
        }
        return *this;
    }

    template <typename T, std::size_t n>
    constexpr aligned(const aligned<T, n>& that) noexcept(std::is_nothrow_constructible_v<Ty, T>)
        : value(that.value), _padding() {}
    template <typename T, std::size_t n>
    constexpr aligned(aligned<T, n>&& that) noexcept(std::is_nothrow_constructible_v<Ty, T>)
        : value(std::move(that.value)), _padding() {}
    template <typename T, std::size_t n>
    constexpr aligned& operator=(const aligned<T, n>& that) noexcept(
        std::is_nothrow_assignable_v<Ty, T>) {
        if constexpr (std::is_trivially_assignable_v<Ty, T>) {
            rtmemcpy(value, that.value);
        }
        else {
            Ty temp(that.value);
            std::swap(value, temp.value);
        }
        return *this;
    }
    template <typename T, std::size_t n>
    constexpr aligned& operator=(aligned<T, n>& that) noexcept(
        std::is_nothrow_assignable_v<Ty, T>) {
        if (this != &that) [[likely]] {
            value = std::move(that.value);
        }
        return *this;
    }

    constexpr ~aligned() noexcept(std::is_nothrow_destructible_v<Ty>) = default;
};

/**
 * @brief Padded type.
 *
 * @tparam Ty Type need to padded.
 * @tparam N Padding
 */
template <typename Ty, std::size_t N>
struct padded {
    Ty value;
    /**
     * @brief Padding
     * For most time, this is not useful.
     */
    std::array<std::byte, N - sizeof(Ty)> _padding;

    constexpr padded() noexcept(std::is_nothrow_destructible_v<Ty>) = default;
    template <typename T>
    requires std::is_constructible_v<Ty, T>
    constexpr padded(T&& value) noexcept(std::is_nothrow_constructible_v<Ty, T>)
        : value(std::forward<T>(value)) {}

    constexpr padded(const padded& that) noexcept(std::is_nothrow_copy_constructible_v<Ty>)
        : value(that.value), _padding() {}
    constexpr padded(padded&& that) noexcept(std::is_nothrow_move_constructible_v<Ty>)
        : value(std::move(that.value)), _padding() {}
    constexpr padded& operator=(const padded& that) {
        if (this != &that) [[likely]] {
            if constexpr (std::is_trivially_copy_assignable_v<Ty>) {
                rtmemcpy(value, that.value);
            }
            else {
                Ty temp(that.value);
                std::swap(value, temp.value);
            }
        }
        return *this;
    }
    constexpr padded& operator=(padded&& that) noexcept(std::is_nothrow_move_assignable_v<Ty>) {
        if (this != &that) [[likely]] {
            value = std::move(that.value);
        }
        return *this;
    }

    template <typename T, std::size_t n>
    constexpr padded(const padded<T, n>& that) noexcept(std::is_nothrow_constructible_v<Ty, T>)
        : value(that.value), _padding() {}
    template <typename T, std::size_t n>
    constexpr padded(padded<T, n>&& that) noexcept(std::is_nothrow_constructible_v<Ty, T>)
        : value(std::move(that.value)), _padding() {}
    template <typename T, std::size_t n>
    constexpr padded& operator=(const padded<T, n>& that) noexcept(
        std::is_nothrow_assignable_v<Ty, T>) {
        if constexpr (std::is_trivially_assignable_v<Ty, T>) {
            rtmemcpy(value, that.value);
        }
        else {
            Ty temp(that.value);
            std::swap(value, temp.value);
        }
        return *this;
    }
    template <typename T, std::size_t n>
    constexpr padded& operator=(padded<T, n>& that) noexcept(std::is_nothrow_assignable_v<Ty, T>) {
        if (this != &that) [[likely]] {
            value = std::move(that.value);
        }
        return *this;
    }

    constexpr ~padded() noexcept(std::is_nothrow_destructible_v<Ty>) = default;
};

} // namespace atom::utils
