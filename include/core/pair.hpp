#pragma once
#include <tuple>
#include <type_traits>
#include <utility>
#include "concepts/type.hpp"
#include "core.hpp"
#include "core/type_traits.hpp"
#include "langdef.hpp"

namespace atom::utils {

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
/**
 * @brief Element container in compressed_pair
 *
 * @tparam Ty Element type
 * @tparam IsFirst Is the first type? A placeholder that could verify the two elements if
 * they have the same type.
 */
template <typename Ty, bool IsFirst>
class NOVTABLE compressed_element {
public:
    using self_type       = compressed_element;
    using value_type      = Ty;
    using reference       = Ty&;
    using const_reference = const Ty&;

    /**
     * @brief Default constructor.
     *
     */
    constexpr compressed_element() noexcept(std::is_nothrow_default_constructible_v<Ty>)
    requires std::is_default_constructible_v<Ty>
    = default;

    template <typename... Args>
    requires std::is_constructible_v<Ty, Args...>
    explicit constexpr compressed_element(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<Ty, Args...>)
        : value_(std::forward<Args>(args)...) {}

    // clang-format off
    constexpr compressed_element(const compressed_element&) 
        noexcept(std::is_nothrow_copy_constructible_v<Ty>)      = default;
    constexpr compressed_element(compressed_element&& that) noexcept(
        std::is_nothrow_move_constructible_v<Ty>) : value_(std::move(that.value_)) {}
    constexpr compressed_element& operator=(const compressed_element&)
        noexcept(std::is_nothrow_copy_assignable_v<Ty>)                    = default;
    constexpr compressed_element& operator=(compressed_element&& that) noexcept(
        std::is_nothrow_move_constructible_v<Ty>) {
        if (this != &that) {
            value_ = std::move(that.value_);
        }
        return *this;
    }
    // clang-format on

    constexpr ~compressed_element() noexcept(std::is_nothrow_destructible_v<Ty>) = default;

    /**
     * @brief Get the value of this element.
     *
     */
    [[nodiscard]] constexpr reference value() noexcept { return value_; }

    /**
     * @brief Get the value of this element.
     *
     */
    [[nodiscard]] constexpr const_reference value() const noexcept { return value_; }

private:
    Ty value_;
};

/**
 * @brief Compressed element. Specific version for pointer type.
 *
 * @tparam Ty Element type.
 */
template <typename Ty, bool IsFirst>
class NOVTABLE compressed_element<Ty*, IsFirst> {
public:
    using self_type       = compressed_element;
    using value_type      = Ty*;
    using reference       = Ty*&;
    using const_reference = const Ty*&;

    constexpr compressed_element() noexcept : value_(nullptr) {}
    explicit constexpr compressed_element(std::nullptr_t) noexcept : value_(nullptr) {}
    template <typename T>
    explicit constexpr compressed_element(T* value) noexcept : value_(value) {}
    constexpr compressed_element(const compressed_element&) noexcept            = default;
    constexpr compressed_element& operator=(const compressed_element&) noexcept = default;
    constexpr ~compressed_element() noexcept                                    = default;

    constexpr compressed_element& operator=(std::nullptr_t) noexcept {
        value_ = nullptr;
        return *this;
    }

    template <typename T>
    constexpr compressed_element& operator=(T* value) noexcept {
        value_ = value;
        return *this;
    }

    constexpr compressed_element(compressed_element&& that) noexcept
        : value_(std::exchange(that.value_, nullptr)) {}

    constexpr compressed_element& operator=(compressed_element&& that) noexcept {
        value_ = std::exchange(that.value_, nullptr);
        return *this;
    }

    /**
     * @brief Get the value of this element.
     *
     */
    [[nodiscard]] constexpr reference value() noexcept { return value_; }

    /**
     * @brief Get the value of this element.
     *
     */
    [[nodiscard]] constexpr auto& value() const noexcept { return value_; }

private:
    Ty* value_;
};

/**
 * @brief When the type is void
 *
 * @tparam IsFirst
 */
template <bool IsFirst>
class NOVTABLE compressed_element<void, IsFirst> {
public:
    using self_type  = compressed_element;
    using value_type = void;

    constexpr compressed_element() noexcept                                = default;
    constexpr compressed_element(const compressed_element&)                = default;
    constexpr compressed_element(compressed_element&&) noexcept            = default;
    constexpr compressed_element& operator=(const compressed_element&)     = default;
    constexpr compressed_element& operator=(compressed_element&&) noexcept = default;
    constexpr ~compressed_element() noexcept                               = default;

    /**
     * @brief You may made something wrong.
     *
     */
    constexpr void value() const noexcept {}
};

template <typename Ret, typename... Args, bool IsFirst>
class NOVTABLE compressed_element<Ret (*)(Args...), IsFirst> {
public:
    using self_type       = compressed_element;
    using value_type      = Ret (*)(Args...);
    using reference       = value_type&;
    using const_reference = const value_type&;

    constexpr explicit compressed_element(const value_type value = nullptr) noexcept
        : value_(value) {}
    constexpr explicit compressed_element(std::nullptr_t) noexcept : value_(nullptr) {}
    constexpr compressed_element(const compressed_element&) noexcept = default;
    constexpr compressed_element(compressed_element&&) noexcept      = default;
    constexpr compressed_element& operator=(std::nullptr_t) noexcept {
        value_ = nullptr;
        return *this;
    }
    constexpr compressed_element& operator=(const compressed_element&) noexcept = default;
    constexpr compressed_element& operator=(compressed_element&& that) noexcept = default;
    constexpr ~compressed_element() noexcept                                    = default;

    /**
     * @brief Get the value of this element.
     *
     */
    constexpr reference value() noexcept { return value_; }

    /**
     * @brief Get the value of this element.
     *
     */
    constexpr const_reference value() const noexcept { return value_; }

private:
    value_type value_;
};
} // namespace internal
/*! @endcond */

template <typename, typename>
class compressed_pair;

template <typename, typename>
class reversed_compressed_pair;

/**
 * @brief Compressed pair. It supports structured binding.
 *
 * EBCO.
 * @tparam First The first element type.
 * @tparam Second The second element type.
 */
template <typename First, typename Second>
class compressed_pair final : private internal::compressed_element<First, true>,
                              private internal::compressed_element<Second, false> {
    using self_type   = compressed_pair;
    using first_base  = internal::compressed_element<First, true>;
    using second_base = internal::compressed_element<Second, false>;

public:
    using first_type  = First;
    using second_type = Second;

    /**
     * @brief Default constructor.
     *
     */
    constexpr compressed_pair() noexcept(
        std::is_nothrow_default_constructible_v<first_base> &&
        std::is_nothrow_default_constructible_v<second_base>)
        : first_base(), second_base() {}

    /**
     * @brief Construct each.
     *
     * @tparam FirstType
     * @tparam SecondType
     */
    template <typename FirstType, typename SecondType>
    constexpr compressed_pair(FirstType&& first, SecondType&& second) noexcept(
        std::is_nothrow_constructible_v<first_base, FirstType> &&
        std::is_nothrow_constructible_v<second_base, SecondType>)
        : first_base(std::forward<FirstType>(first)),
          second_base(std::forward<SecondType>(second)) {}

    template <typename Tuple1, typename Tuple2, size_t... Is1, size_t... Is2>
    constexpr compressed_pair(Tuple1& t1, Tuple2& t2, std::index_sequence<Is1...>, std::index_sequence<Is2...>) noexcept(
        std::is_nothrow_constructible_v<First, std::tuple_element_t<Is1, Tuple1>...> &&
        std::is_nothrow_constructible_v<Second, std::tuple_element_t<Is2, Tuple2>...>)
        : first_base(std::get<Is1>(std::move(t1))...),
          second_base(std::get<Is2>(std::move(t2))...) {}

    template <typename... Tys1, typename... Tys2>
    constexpr compressed_pair(std::piecewise_construct_t, std::tuple<Tys1...> t1, std::tuple<Tys2...> t2) noexcept(
        noexcept(compressed_pair(
            t1, t2, std::index_sequence_for<Tys1...>{}, std::index_sequence_for<Tys2...>{})))
        : compressed_pair(
              t1, t2, std::index_sequence_for<Tys1...>{}, std::index_sequence_for<Tys2...>{}) {}

    // template <
    //     typename That, typename = std::enable_if_t<!std::is_same_v<
    //                        compressed_pair, std::remove_cv_t<std::remove_reference_t<That>>>>>
    // constexpr compressed_pair(That&& that) noexcept(
    //     std::is_nothrow_constructible_v<First, same_reference_t<First, That>> &&
    //     std::is_nothrow_constructible_v<Second, same_reference_t<Second, That>>)
    //     : first_base([&] {
    //           if constexpr (concepts::public_pair<That>) {
    //               return std::forward<That>(that).first;
    //           }
    //           else if constexpr (concepts::private_pair<That>) {
    //               return std::forward<That>(that).first();
    //           }
    //           else {
    //               static_assert(false, "no suitable way to construct the first element");
    //           }
    //       }),
    //       second_base([&] {
    //           if constexpr (concepts::public_pair<That>) {
    //               return std::forward<That>(that).second;
    //           }
    //           else if constexpr (concepts::private_pair<That>) {
    //               return std::forward<That>(that).second();
    //           }
    //           else {
    //               static_assert(false, "no suitable way to construct the second element");
    //           }
    //       }) {}

    constexpr compressed_pair(const compressed_pair&)            = default;
    constexpr compressed_pair& operator=(const compressed_pair&) = default;

    constexpr compressed_pair(compressed_pair&& that) noexcept(
        std::is_nothrow_move_constructible_v<first_base> &&
        std::is_nothrow_move_constructible_v<second_base>)
        // : first_base(std::move(static_cast<first_base&&>(that))),
        //   second_base(std::move(static_cast<second_base&&>(that))) {}
        : first_base(static_cast<first_base&&>(std::move(that))),
          second_base(static_cast<second_base&&>(std::move(that))) {}

    constexpr compressed_pair& operator=(compressed_pair&& that) noexcept(
        std::is_nothrow_move_assignable_v<first_base> &&
        std::is_nothrow_move_assignable_v<second_base>) {
        static_cast<first_base&>(*this)  = static_cast<first_base&&>(std::move(that));
        static_cast<second_base&>(*this) = static_cast<second_base&&>(std::move(that));
        return *this;
    }

    constexpr ~compressed_pair() noexcept(
        std::is_nothrow_destructible_v<first_base> &&
        std::is_nothrow_destructible_v<second_base>) = default;

    [[nodiscard]] constexpr First& first() noexcept {
        return static_cast<first_base&>(*this).value();
    }

    [[nodiscard]] constexpr const First& first() const noexcept {
        return static_cast<const first_base&>(*this).value();
    }

    [[nodiscard]] constexpr Second& second() noexcept {
        return static_cast<second_base&>(*this).value();
    }

    [[nodiscard]] constexpr const Second& second() const noexcept {
        return static_cast<const second_base&>(*this).value();
    }

    template <
        typename = std::enable_if_t<
            std::is_copy_constructible_v<First> && std::is_copy_constructible_v<Second>>>
    constexpr operator std::pair<First, Second>() noexcept(
        std::is_nothrow_copy_constructible_v<First> &&
        std::is_nothrow_copy_constructible_v<Second>) {
        return std::pair<First, Second>(first(), second());
    }
};

// if it returns true, two `compressed_pair`s must have the same tparams.
template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
[[nodiscard]] constexpr inline bool operator==(
    const compressed_pair<LFirst, LSecond>& lhs,
    const compressed_pair<RFirst, RSecond>& rhs) noexcept {
    if constexpr (std::is_same_v<LFirst, RFirst> && std::is_same_v<LSecond, RSecond>) {
        return lhs.first() == rhs.first() && lhs.second() && rhs.second();
    }
    else {
        return false;
    }
}

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
[[nodiscard]] constexpr inline bool operator!=(
    const compressed_pair<LFirst, LSecond>& lhs,
    const compressed_pair<RFirst, RSecond>& rhs) noexcept {
    return !(lhs == rhs);
}

template <typename Ty, typename First, typename Second>
[[nodiscard]] constexpr inline bool operator==(
    const compressed_pair<First, Second>& pair, const Ty& val) noexcept {
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
[[nodiscard]] constexpr inline bool operator!=(
    const compressed_pair<First, Second>& pair, const Ty& val) noexcept {
    return !(pair == val);
}

/**
 * @brief Reversed version of `compressed_pair`.
 *
 * The storage layout of data inside it is consistent with `compressed_pair`, just the opposite in
 * logic.
 * @tparam First The first type.
 * @tparam Second The second type.
 */
template <typename First, typename Second>
class reversed_compressed_pair final
    : private ::atom::utils::internal::compressed_element<Second, true>,
      private ::atom::utils::internal::compressed_element<First, false> {
    using self_type   = reversed_compressed_pair;
    using first_base  = internal::compressed_element<Second, true>;
    using second_base = internal::compressed_element<First, false>;

public:
    using first_type  = First;
    using second_type = Second;

    constexpr reversed_compressed_pair() noexcept(
        std::is_nothrow_default_constructible_v<second_base> &&
        std::is_nothrow_default_constructible_v<first_base>)
        : first_base(), second_base() {}

    template <typename FirstType, typename SecondType>
    constexpr reversed_compressed_pair(FirstType&& first, SecondType&& second) noexcept(
        std::is_nothrow_constructible_v<second_base, SecondType> &&
        std::is_nothrow_constructible_v<first_base, FirstType>)
        : first_base(std::forward<SecondType>(second)),
          second_base(std::forward<FirstType>(first)) {}

    template <typename Tuple1, typename Tuple2, size_t... Is1, size_t... Is2>
    constexpr reversed_compressed_pair(Tuple1& t1, Tuple2& t2, std::index_sequence<Is1...>, std::index_sequence<Is2...>) noexcept(
        std::is_nothrow_constructible_v<First, std::tuple_element_t<Is1, Tuple1>...> &&
        std::is_nothrow_constructible_v<Second, std::tuple_element_t<Is2, Tuple2>...>)
        : first_base(std::get<Is1>(std::move(t1))...),
          second_base(std::get<Is2>(std::move(t2))...) {}

    template <typename... Tys1, typename... Tys2>
    constexpr reversed_compressed_pair(std::piecewise_construct_t, std::tuple<Tys1...> t1, std::tuple<Tys2...> t2) noexcept(
        noexcept(reversed_compressed_pair(
            t1, t2, std::index_sequence_for<Tys1...>{}, std::index_sequence_for<Tys2...>{})))
        : reversed_compressed_pair(
              t1, t2, std::index_sequence_for<Tys1...>{}, std::index_sequence_for<Tys2...>{}) {}

    constexpr reversed_compressed_pair(const reversed_compressed_pair&)            = default;
    constexpr reversed_compressed_pair& operator=(const reversed_compressed_pair&) = default;

    constexpr reversed_compressed_pair(reversed_compressed_pair&& that) noexcept(
        std::is_nothrow_move_constructible_v<first_base> &&
        std::is_nothrow_move_constructible_v<second_base>)
        : first_base(std::move(static_cast<first_base&>(that))),
          second_base(std::move(static_cast<second_base&>(that))) {}

    constexpr reversed_compressed_pair& operator=(reversed_compressed_pair&& that) noexcept(
        std::is_nothrow_move_assignable_v<first_base> &&
        std::is_nothrow_move_assignable_v<second_base>) {
        first()  = std::move(that.first());
        second() = std::move(that.second());
        return *this;
    }

    constexpr ~reversed_compressed_pair() noexcept(
        std::is_nothrow_destructible_v<first_base> &&
        std::is_nothrow_destructible_v<second_base>) = default;

    [[nodiscard]] constexpr First& first() noexcept {
        return static_cast<second_base&>(*this).value();
    }

    [[nodiscard]] constexpr const First& first() const noexcept {
        return static_cast<const second_base&>(*this).value();
    }

    [[nodiscard]] constexpr Second& second() noexcept {
        return static_cast<first_base&>(*this).value();
    }

    [[nodiscard]] constexpr const Second& second() const noexcept {
        return static_cast<const first_base&>(*this).value();
    }
};

// if it returns true, two `reversed_compressed_pair`s must have the same tparams.
template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr inline bool operator==(
    const reversed_compressed_pair<LFirst, LSecond>& lhs,
    const reversed_compressed_pair<RFirst, RSecond>& rhs) {
    if constexpr (std::is_same_v<LFirst, RFirst> && std::is_same_v<LFirst, RFirst>) {
        return lhs.first() == rhs.first();
    }
    else {
        return false;
    }
}

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr inline bool operator!=(
    const reversed_compressed_pair<LFirst, LSecond>& lhs,
    const reversed_compressed_pair<RFirst, RSecond>& rhs) {
    return !(lhs == rhs);
}

template <typename Ty, typename First, typename Second>
constexpr inline bool operator==(
    const Ty& lhs, const reversed_compressed_pair<First, Second>& rhs) {
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
constexpr inline bool operator!=(
    const Ty& lhs, const reversed_compressed_pair<First, Second>& rhs) {
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
constexpr inline bool operator==(
    const reversed_pair<LFirst, LSecond>& lhs, const reversed_pair<RFirst, RSecond>& rhs) {
    if constexpr (std::is_same_v<LFirst, RFirst> && std::is_same_v<LSecond, RSecond>) {
        return lhs.second == rhs.second && lhs.first == rhs.first;
    }
    else {
        return false;
    }
}

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr inline bool operator!=(
    const reversed_pair<LFirst, LSecond>& lhs, const reversed_pair<RFirst, RSecond>& rhs) {
    return !(lhs == rhs);
}

template <typename Ty, typename First, typename Second>
constexpr inline bool operator==(const Ty& lhs, const reversed_pair<First, Second>& rhs) {
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
constexpr inline bool operator!=(const Ty& lhs, const reversed_pair<First, Second>& rhs) {
    return !(lhs == rhs);
}

template <
    typename First, typename Second, template <typename, typename> typename Pair = compressed_pair>
struct pair {
public:
    using value_type = Pair<First, Second>;

    template <typename... Args>
    requires std::is_constructible_v<value_type, Args...>
    pair(Args&&... args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>)
        : pair_(std::forward<Args>(args)...) {}

    pair(const pair& that) noexcept(std::is_nothrow_copy_constructible_v<value_type>)
        : pair_(that.pair_) {}

    pair(pair&& that) noexcept(std::is_nothrow_move_constructible_v<value_type>)
        : pair_(std::move(that.pair_)) {}

    template <typename... Tys1, typename... Tys2>
    pair(std::piecewise_construct_t, std::tuple<Tys1...> t1, std::tuple<Tys2...> t2) noexcept(
        std::is_nothrow_constructible_v<
            value_type, std::piecewise_construct_t, std::tuple<Tys1...>, std::tuple<Tys2...>>)
        : pair(std::piecewise_construct, std::move(t1), std::move(t2)) {}

    pair& operator=(const pair& that) noexcept(std::is_nothrow_copy_assignable_v<value_type>) {
        if (this != &that) [[likely]] {
            pair_ = that.pair_;
        }
        return *this;
    }

    pair& operator=(pair&& that) noexcept(std::is_nothrow_move_assignable_v<value_type>) {
        if (this != &that) [[likely]] {
            pair_ = that.pair_;
        }
        return *this;
    }

    ~pair() noexcept(std::is_nothrow_destructible_v<value_type>) = default;

    constexpr First& first() noexcept {
        if constexpr (requires { pair_.first; }) {
            return pair_.first;
        }
        else if constexpr (requires { pair_.first(); }) {
            return pair_.first();
        }
        else {
            static_assert(false, "No valid way to get the first value.");
        }
    }

    constexpr const First& first() const noexcept {
        if constexpr (requires { pair_.first; }) {
            return pair_.first;
        }
        else if constexpr (requires { pair_.first(); }) {
            return pair_.first();
        }
        else {
            static_assert(false, "No valid way to get the first value.");
        }
    }

    constexpr Second& second() noexcept {
        if constexpr (requires { pair_.second; }) {
            return pair_.second;
        }
        else if constexpr (requires { pair_.second(); }) {
            return pair_.second();
        }
        else {
            static_assert(false, "No valid way to get the second value.");
        }
    }

    constexpr const Second& second() const noexcept {
        if constexpr (requires { pair_.second; }) {
            return pair_.second;
        }
        else if constexpr (requires { pair_.second(); }) {
            return pair_.second();
        }
        else {
            static_assert(false, "No valid way to get the second value.");
        }
    }

    constexpr bool operator==(const pair& that) const noexcept { return pair_ == that.pair_; }

    template <typename Ty>
    constexpr bool operator==(const Ty& that) const noexcept {
        if constexpr (requires { pair_ == that; }) {
            return pair_ == that;
        }
        else {
            return false;
        }
    }

    constexpr bool operator!=(const pair& that) const noexcept { return pair_ != that.pair_; }

    template <typename Ty>
    constexpr bool operator!=(const Ty& that) const noexcept {
        if constexpr (requires { pair_ != that; }) {
            return pair_ != that;
        }
        else {
            return false;
        }
    }

private:
    Pair<First, Second> pair_;
};

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

template <typename First, typename Second, template <typename, typename> typename Pair>
struct reversed_result<pair<First, Second, Pair>> {
    using type = pair<Second, First, Pair>;
};

template <typename Pair>
using reversed_result_t = typename reversed_result<Pair>::type;

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
template <typename Pair>
concept reversible_pair = requires {
    typename reversed_result<Pair>::type;
    typename Pair::first_type;
    typename Pair::second_type;
} && std::is_trivial_v<typename Pair::first_type> && std::is_trivial_v<typename Pair::second_type>;
} // namespace internal
/*! @endcond */

///////////////////////////////////////////////////////////////////////////////
// reverse
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get the reversed pair.
 *
 * @tparam Pair The pair type. This tparam could be deduced automatically.
 * @param pair The pair need to reverse.
 * @return Reversed pair.
 */
template <typename Pair>
requires internal::reversible_pair<std::remove_cv_t<Pair>>
constexpr inline decltype(auto) reverse(Pair& pair) noexcept {
    using result_type = typename UTILS same_cv_t<reversed_result_t<std::remove_cv_t<Pair>>, Pair>;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<result_type&>(pair);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}

///////////////////////////////////////////////////////////////////////////////
// support for structured bingings
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Get the element at the Index.
 *
 * @tparam First The first element type of this compressed_pair.
 * @tparam Second The second element type of this compressed_pair.
 * @param pair Instance.
 * @return Element reference.
 */
template <size_t Index, typename First, typename Second>
constexpr inline auto& get(::atom::utils::compressed_pair<First, Second>& pair) noexcept {
    static_assert(Index < 2, "Pair doesn't contains so many elements.");
    if constexpr (Index == 0U) {
        return pair.first();
    }
    else {
        return pair.second();
    }
}

/**
 * @brief Get the element at the Index.
 *
 * @tparam First The first element type of this compressed_pair.
 * @tparam Second The second element type of this compressed_pair.
 * @param pair Instance.
 * @return Element reference.
 */
template <size_t Index, typename First, typename Second>
constexpr inline const auto& get(
    const ::atom::utils::compressed_pair<First, Second>& pair) noexcept {
    static_assert(Index < 2, "Pair doesn't contains so many elements.");
    if constexpr (Index == 0U) {
        return pair.first();
    }
    else {
        return pair.second();
    }
}

/**
 * @brief Get the element at the Index.
 *
 * @tparam First The first element type of this compressed_pair.
 * @tparam Second The second element type of this compressed_pair.
 * @param pair Instance.
 * @return Element reference.
 */
template <size_t Index, typename First, typename Second>
constexpr inline auto& get(::atom::utils::reversed_compressed_pair<First, Second>& pair) noexcept {
    static_assert(Index < 2, "Pair doesn't contains so many elements.");
    if constexpr (Index == 0U) {
        return pair.first();
    }
    else {
        return pair.second();
    }
}

/**
 * @brief Get the element at the Index.
 *
 * @tparam First The first element type of this compressed_pair.
 * @tparam Second The second element type of this compressed_pair.
 * @param pair Instance.
 * @return Element reference.
 */
template <size_t Index, typename First, typename Second>
constexpr inline const auto& get(
    const ::atom::utils::reversed_compressed_pair<First, Second>& pair) noexcept {
    static_assert(Index < 2, "Pair doesn't contains so many elements.");
    if constexpr (Index == 0U) {
        return pair.first();
    }
    else {
        return pair.second();
    }
}

template <
    size_t Index, typename First, typename Second, template <typename, typename> typename Pair>
constexpr inline auto& get(pair<First, Second, Pair>& pair) noexcept {
    static_assert(Index < 2, "Index out of range");
    if constexpr (Index) {
        return pair.second();
    }
    else {
        return pair.first();
    }
}

template <
    size_t Index, typename First, typename Second, template <typename, typename> typename Pair>
constexpr inline const auto& get(const pair<First, Second, Pair>& pair) noexcept {
    static_assert(Index < 2, "Index out of range");
    if constexpr (Index) {
        return pair.second();
    }
    else {
        return pair.first();
    }
}

} // namespace atom::utils

/*! @cond TURN_OFF_DOXYGEN */
namespace std {

template <typename First, typename Second>
struct tuple_size<::atom::utils::compressed_pair<First, Second>>
    : std::integral_constant<size_t, 2> {};

template <typename First, typename Second>
struct tuple_size<::atom::utils::reversed_compressed_pair<First, Second>>
    : std::integral_constant<size_t, 2> {};

template <typename First, typename Second, template <typename, typename> typename Pair>
struct tuple_size<::atom::utils::pair<First, Second, Pair>> : std::integral_constant<size_t, 2> {};

template <size_t Index, typename First, typename Second>
struct tuple_element<Index, ::atom::utils::compressed_pair<First, Second>> {
    static_assert(Index < 2);
    using type = std::conditional_t<(Index == 0), First, Second>;
};

template <size_t Index, typename First, typename Second>
struct tuple_element<Index, ::atom::utils::reversed_compressed_pair<First, Second>> {
    static_assert(Index < 2);
    using type = std::conditional_t<(Index == 0), First, Second>;
};

template <
    size_t Index, typename First, typename Second, template <typename, typename> typename Pair>
struct tuple_element<Index, ::atom::utils::pair<First, Second, Pair>> {
    static_assert(Index < 2);
    using type = std::conditional_t<(Index == 0), First, Second>;
};

} // namespace std
/*! @endcond */
