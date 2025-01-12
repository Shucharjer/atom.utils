#pragma once
#include <type_traits>
#include <utility>
#include "type_traits.h"
#include "utils_macro.h"

namespace atom::utils {

namespace internal {
/**
 * @brief Element container in compressed_pair
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

    constexpr compressed_element& operator=(nullptr_t) noexcept {
        value_ = nullptr;
        return *this;
    }

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

    constexpr explicit compressed_element(const value_type value = nullptr) noexcept
        : value_(value) {}
    constexpr explicit compressed_element(nullptr_t) noexcept : value_(nullptr) {}
    constexpr compressed_element(const compressed_element&) noexcept = default;
    constexpr compressed_element(compressed_element&&) noexcept      = default;
    constexpr compressed_element& operator=(nullptr_t) noexcept {
        value_ = nullptr;
        return *this;
    }
    constexpr compressed_element& operator=(const compressed_element&) noexcept = default;
    constexpr compressed_element& operator=(compressed_element&& that) noexcept = default;
    constexpr ~compressed_element() noexcept                                    = default;

    constexpr reference get() noexcept { return value_; }
    constexpr const_reference get() const noexcept { return value_; }

private:
    value_type value_;
};
} // namespace internal

template <typename, typename>
class compressed_pair;

template <typename, typename>
class reversed_compressed_pair;

struct placeholder_t {};

constexpr inline placeholder_t placeholder{};

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
    constexpr explicit compressed_pair(
        FirstType&& first, SecondType&& second
    ) noexcept(std::is_nothrow_constructible_v<first_base, FirstType> && std::is_nothrow_constructible_v<second_base, SecondType>)
        : first_base(std::forward<FirstType>(first)),
          second_base(std::forward<SecondType>(second)) {}

    template <typename Ty>
    constexpr explicit compressed_pair(
        Ty&& val, placeholder_t
    ) noexcept(std::is_nothrow_constructible_v<first_base, Ty> && std::is_nothrow_default_constructible_v<second_base>)
        : first_base(std::forward<Ty>(val)), second_base() {}

    template <typename Ty>
    constexpr explicit compressed_pair(
        placeholder_t, Ty&& val
    ) noexcept(std::is_nothrow_default_constructible_v<first_base> && std::is_nothrow_constructible_v<second_base, Ty>)
        : first_base(), second_base(std::forward<Ty>(val)) {}

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

// compressed_pair<int, char> : first_type -> int, second_type -> char
// reversed_compressed_pair<char, int> : first_type -> char, second_type -> int
template <typename First, typename Second>
class reversed_compressed_pair final : private internal::compressed_element<Second, true>,
                                       private internal::compressed_element<First, false> {
public:
    using self_type   = reversed_compressed_pair;
    using first_base  = internal::compressed_element<Second, true>;
    using second_base = internal::compressed_element<First, false>;

    constexpr reversed_compressed_pair(
    ) noexcept(std::is_nothrow_default_constructible_v<second_base> && std::is_nothrow_default_constructible_v<first_base>)
        : first_base(), second_base() {}

    template <typename FirstType, typename SecondType>
    constexpr explicit reversed_compressed_pair(
        FirstType&& first, SecondType&& second
    ) noexcept(std::is_nothrow_constructible_v<second_base, SecondType> && std::is_nothrow_constructible_v<first_base, FirstType>)
        : first_base(std::forward<SecondType>(second)),
          second_base(std::forward<FirstType>(first)) {}

    template <typename Ty>
    constexpr explicit reversed_compressed_pair(
        Ty&& val, placeholder_t
    ) noexcept(std::is_nothrow_constructible_v<first_base, Ty> && std::is_nothrow_default_constructible_v<second_base>)
        : first_base(std::forward<Ty>(val)), second_base() {}

    template <typename Ty>
    constexpr explicit reversed_compressed_pair(
        placeholder_t, Ty&& val
    ) noexcept(std::is_nothrow_default_constructible_v<first_base> && std::is_nothrow_constructible_v<second_base, Ty>)
        : first_base(), second_base(std::forward<Ty>(val)) {}

    constexpr reversed_compressed_pair(const reversed_compressed_pair&)            = default;
    constexpr reversed_compressed_pair& operator=(const reversed_compressed_pair&) = default;

    constexpr reversed_compressed_pair(reversed_compressed_pair&& that
    ) noexcept(std::is_nothrow_move_constructible_v<first_base> && std::is_nothrow_move_constructible_v<second_base>)
        : first_base(std::move(static_cast<first_base&>(that))),
          second_base(std::move(static_cast<second_base&>(that))) {}

    constexpr reversed_compressed_pair& operator=(reversed_compressed_pair&& that
    ) noexcept(std::is_nothrow_move_assignable_v<first_base> && std::is_nothrow_move_assignable_v<second_base>) {
        first()  = std::move(that.first());
        second() = std::move(that.second());
        return *this;
    }

    constexpr ~reversed_compressed_pair(
    ) noexcept(std::is_nothrow_destructible_v<first_base> && std::is_nothrow_destructible_v<second_base>) =
        default;

    [[nodiscard]] constexpr First& first() noexcept {
        return static_cast<second_base&>(*this).get();
    }

    [[nodiscard]] constexpr const First& first() const noexcept {
        return static_cast<const second_base&>(*this).get();
    }

    [[nodiscard]] constexpr Second& second() noexcept {
        return static_cast<first_base&>(*this).get();
    }

    [[nodiscard]] constexpr const Second& second() const noexcept {
        return static_cast<const first_base&>(*this).get();
    }
};

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr bool operator==(
    const reversed_compressed_pair<LFirst, LSecond>& lhs,
    const reversed_compressed_pair<RFirst, RSecond>& rhs
) {
    if constexpr (std::is_same_v<LFirst, RFirst> && std::is_same_v<LFirst, RFirst>) {
        return lhs.first() == rhs.first();
    }
    else {
        return false;
    }
}

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr bool operator!=(
    const reversed_compressed_pair<LFirst, LSecond>& lhs,
    const reversed_compressed_pair<RFirst, RSecond>& rhs
) {
    return !(lhs == rhs);
}

template <typename Ty, typename First, typename Second>
constexpr bool operator==(const Ty& lhs, const reversed_compressed_pair<First, Second>& rhs) {
    if constexpr (std::is_convertible_v<Ty, First>) {
        return lhs == rhs.first();
    }
    else if constexpr (std::is_convertible_v<Ty, Second>) {
        return lhs == rhs.second();
    }
    else {
        return false;
    }
}

template <typename Ty, typename First, typename Second>
constexpr bool operator!=(const Ty& lhs, const reversed_compressed_pair<First, Second>& rhs) {
    return !(lhs == rhs);
}

template <typename First, typename Second>
struct reversed_pair {
    using first_type  = Second;
    using second_type = First;

    first_type second;
    second_type first;
};

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr bool operator==(
    const reversed_pair<LFirst, LSecond>& lhs, const reversed_pair<RFirst, RSecond>& rhs
) {
    if constexpr (std::is_same_v<LFirst, RFirst> && std::is_same_v<LSecond, RSecond>) {
        return lhs.second == rhs.second && lhs.first == rhs.first;
    }
    else {
        return false;
    }
}

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr bool operator!=(
    const reversed_pair<LFirst, LSecond>& lhs, const reversed_pair<RFirst, RSecond>& rhs
) {
    return !(lhs == rhs);
}

template <typename Ty, typename First, typename Second>
constexpr bool operator==(const Ty& lhs, const reversed_pair<First, Second>& rhs) {
    if constexpr (std::is_convertible_v<Ty, First>) {
        return rhs.first == lhs;
    }
    else if constexpr (std::is_convertible_v<Ty, Second>) {
        return rhs.second == lhs;
    }
    else {
        return false;
    }
}

template <typename Ty, typename First, typename Second>
constexpr bool operator!=(const Ty& lhs, const reversed_pair<First, Second>& rhs) {
    return !(lhs == rhs);
}

template <typename>
struct reversed_result;

template <typename First, typename Second>
struct reversed_result<compressed_pair<First, Second>> {
    using type = reversed_compressed_pair<Second, First>;
};

template <typename First, typename Second>
struct reversed_result<reversed_compressed_pair<First, Second>> {
    using type = compressed_pair<Second, First>;
};

template <typename First, typename Second>
struct reversed_result<std::pair<First, Second>> {
    using type = reversed_pair<Second, First>;
};

template <typename First, typename Second>
struct reversed_result<reversed_pair<Second, First>> {
    using type = std::pair<Second, First>;
};

template <typename Pair>
using reversed_result_t = typename reversed_result<Pair>::type;

template <typename Pair>
concept reversible_pair = requires { typename reversed_result<Pair>::type; };

/**
 * @brief Get the reversed pair.
 *
 * @tparam Pair The pair type. This tparam could be deduced automatically.
 * @param pair The pair need to reverse.
 * @return Reversed pair.
 */
template <typename Pair>
requires reversible_pair<std::remove_cv_t<Pair>>
constexpr decltype(auto) reverse(Pair& pair) noexcept {
    using result_type = typename UTILS same_cv_t<reversed_result_t<std::remove_cv_t<Pair>>, Pair>;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<result_type&>(pair);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}

} // namespace atom::utils
