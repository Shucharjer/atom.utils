#pragma once
#include <type_traits>
#include <utility>
#include "utils_macro.h"

namespace atom::utils {

namespace internal {
/**
 * @brief Element container in copressed_pair
 *
 * @tparam Ty Element type
 * @tparam IsFirst Is the first type? A placeholder that could verify the two elements if
 * they have the same type.
 */
template <typename Ty, bool IsFirst>
requires(!std::is_reference_v<Ty>)
class compressed_element {
public:
    using self_type       = compressed_element;
    using value_type      = Ty;
    using reference       = Ty&;
    using const_reference = const Ty&;

    template <typename = std::void_t<>>
    requires std::is_default_constructible_v<Ty>
    constexpr compressed_element() noexcept(std::is_nothrow_default_constructible_v<Ty>) {}

    template <typename T>
    requires(!std::is_same_v<T, self_type>)
    explicit constexpr compressed_element(T&& val) noexcept(std::is_nothrow_constructible_v<Ty, T>)
        : value_(std::forward<T>(val)) {}

    // clang-format off
    constexpr compressed_element(const compressed_element&) 
        noexcept(std::is_nothrow_copy_constructible_v<Ty>)      = default;
    constexpr compressed_element(compressed_element&&) noexcept = default;
    constexpr compressed_element& operator=(const compressed_element&)
        noexcept(std::is_nothrow_copy_assignable_v<Ty>)                    = default;
    constexpr compressed_element& operator=(compressed_element&&) noexcept = default;
    // clang-format on

    constexpr ~compressed_element() noexcept(std::is_nothrow_destructible_v<Ty>) = default;

    [[nodiscard]] constexpr reference get() noexcept { return value_; }

    [[nodiscard]] constexpr const_reference get() const noexcept { return value_; }

private:
    Ty value_;
};

/**
 * @brief Compressed element. Specific version for pointer type.
 *
 * @tparam Ty Element type.
 */
template <typename Ty, bool IsFirst>
class compressed_element<Ty*, IsFirst> {
public:
    using self_type       = compressed_element;
    using value_type      = Ty*;
    using reference       = Ty*&;
    using const_reference = const Ty*&;

    constexpr compressed_element() noexcept : value_(nullptr) {}
    explicit constexpr compressed_element(nullptr_t) noexcept : value_(nullptr) {}
    constexpr compressed_element(const compressed_element&) noexcept            = default;
    constexpr compressed_element& operator=(const compressed_element&) noexcept = default;
    constexpr ~compressed_element() noexcept                                    = default;

    constexpr compressed_element& operator=(nullptr_t) noexcept { value_ = nullptr; }

    constexpr compressed_element(compressed_element&& that) noexcept
        : value_(std::exchange(that.value_, nullptr)) {}

    constexpr compressed_element& operator=(compressed_element&& that) noexcept {
        value_ = std::exchange(that.value_, nullptr);
        return *this;
    }

    [[nodiscard]] constexpr reference get() noexcept { return value_; }

    [[nodiscard]] constexpr auto& get() const noexcept { return value_; }

private:
    Ty* value_;
};

/**
 * @brief When the type is void
 *
 * @tparam IsFirst
 */
template <bool IsFirst>
class compressed_element<void, IsFirst> {
public:
    using self_type  = compressed_element;
    using value_type = void;

    constexpr compressed_element() noexcept                                = default;
    constexpr compressed_element(const compressed_element&)                = default;
    constexpr compressed_element(compressed_element&&) noexcept            = default;
    constexpr compressed_element& operator=(const compressed_element&)     = default;
    constexpr compressed_element& operator=(compressed_element&&) noexcept = default;
    constexpr ~compressed_element() noexcept                               = default;

    constexpr void get() const noexcept {}
};

template <typename Ret, typename... Args, bool IsFirst>
class compressed_element<Ret (*)(Args...), IsFirst> {
public:
    using self_type       = compressed_element;
    using value_type      = Ret (*)(Args...);
    using reference       = value_type&;
    using const_reference = const value_type&;

    constexpr compressed_element(value_type value = nullptr) noexcept : value_(value) {}
    constexpr compressed_element(nullptr_t) noexcept : value_(nullptr) {}
    constexpr compressed_element(const compressed_element&) noexcept = default;
    constexpr compressed_element(compressed_element&&) noexcept      = default;
    constexpr compressed_element& operator=(nullptr_t) noexcept { value_ = nullptr; }
    constexpr compressed_element& operator=(const compressed_element&) noexcept = default;
    constexpr compressed_element& operator=(compressed_element&& that) noexcept = default;
    constexpr ~compressed_element() noexcept                                    = default;

    constexpr reference get() noexcept { return value_; }
    constexpr const_reference get() const noexcept { return value_; }

private:
    value_type value_;
};
} // namespace internal

template <typename First, typename Second>
class compressed_pair final : private UTILS internal::compressed_element<First, true>,
                              private UTILS internal::compressed_element<Second, false> {
public:
    using self_type   = compressed_pair;
    using first_base  = UTILS internal::compressed_element<First, true>;
    using second_base = UTILS internal::compressed_element<Second, false>;

    constexpr compressed_pair(
    ) noexcept(std::is_nothrow_default_constructible_v<first_base> && std::is_nothrow_default_constructible_v<second_base>)
        : first_base(), second_base() {}

    template <typename FirstType, typename SecondType>
    constexpr compressed_pair(
        FirstType&& first, SecondType&& second
    ) noexcept(std::is_nothrow_constructible_v<first_base, FirstType> && std::is_nothrow_constructible_v<second_base, SecondType>)
        : first_base(std::forward<FirstType>(first)),
          second_base(std::forward<SecondType>(second)) {}

    constexpr compressed_pair(const compressed_pair&)            = default;
    constexpr compressed_pair& operator=(const compressed_pair&) = default;

    constexpr compressed_pair(compressed_pair&& that
    ) noexcept(std::is_nothrow_move_constructible_v<first_base> && std::is_nothrow_move_constructible_v<second_base>)
        : first_base(std::move(static_cast<first_base&>(that))),
          second_base(std::move(static_cast<second_base&>(that))) {}

    constexpr compressed_pair& operator=(compressed_pair&& that
    ) noexcept(std::is_nothrow_move_assignable_v<first_base> && std::is_nothrow_move_assignable_v<second_base>) {
        first()  = std::move(that.first());
        second() = std::move(that.second());
        return *this;
    }

    constexpr ~compressed_pair(
    ) noexcept(std::is_nothrow_destructible_v<first_base> && std::is_nothrow_destructible_v<second_base>) =
        default;

    [[nodiscard]] constexpr First& first() noexcept {
        return static_cast<first_base&>(*this).get();
    }

    [[nodiscard]] constexpr const First& first() const noexcept {
        return static_cast<const first_base&>(*this).get();
    }

    [[nodiscard]] constexpr Second& second() noexcept {
        return static_cast<second_base&>(*this).get();
    }

    [[nodiscard]] constexpr const Second& second() const noexcept {
        return static_cast<const second_base&>(*this).get();
    }

    [[nodiscard]] constexpr auto to_pair() -> std::pair<First, Second> {
        return std::make_pair<First&, Second&>(first(), second());
    }

    [[nodiscard]] constexpr auto to_pair() const -> const std::pair<const First&, const Second&> {
        return std::make_pair<const First&, const Second&>(first(), second());
    }
};

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
[[nodiscard]] constexpr bool operator==(
    const compressed_pair<LFirst, LSecond>& lhs, const compressed_pair<RFirst, RSecond>& rhs
) noexcept {
    if constexpr (std::is_same_v<LFirst, RFirst> && std::is_same_v<LSecond, RSecond>) {
        return lhs.first() == rhs.first() && lhs.second() && rhs.second();
    }
    else {
        return false;
    }
}

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
[[nodiscard]] constexpr bool operator!=(
    const compressed_pair<LFirst, LSecond>& lhs, const compressed_pair<RFirst, RSecond>& rhs
) noexcept {
    return !(lhs == rhs);
}

template <typename Ty, typename First, typename Second>
[[nodiscard]] constexpr bool operator==(
    const compressed_pair<First, Second>& pair, const Ty& val
) noexcept {
    if constexpr (std::is_convertible_v<Ty, First>) {
        return pair.first() == val;
    }
    else if constexpr (std::is_convertible_v<Ty, Second>) {
        return pair.second() == val;
    }
    else {
        return false;
    }
}

template <typename Ty, typename First, typename Second>
[[nodiscard]] constexpr bool operator!=(
    const compressed_pair<First, Second>& pair, const Ty& val
) noexcept {
    return !(pair == val);
}

} // namespace atom::utils
