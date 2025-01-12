#pragma once
#include <concepts>
#include <functional>
#include <type_traits>
#include "utils_macro.h"

namespace atom::utils {

template <std::integral auto Integral>
struct is_zero : std::false_type {};

template <>
struct is_zero<0> : std::true_type {};

template <std::integral auto Integral>
constexpr bool is_zero_v = is_zero<Integral>::value;

template <std::integral auto Integral>
constexpr bool is_positive_integral_v = std::integral_constant<bool, (Integral > 0)>::value;

template <std::integral auto Integral>
constexpr bool is_negative_integral_v = std::integral_constant<bool, (Integral < 0)>::value;

template <typename To, typename From>
struct same_constness {
    using type = To;
};

template <typename To, typename From>
struct same_constness<To, From const> {
    using type = To const;
};

template <typename To, typename From>
using same_constness_t = typename same_constness<To, From>::type;

constexpr bool is_pow_two(const std::size_t num) { return num && !(num & (num - 1)); }

template <typename Ty>
using cast_to_void_pointer_t = void*;

template <typename Ty>
struct is_tuple : std::false_type {};

template <typename... Args, template <typename...> typename Tuple>
struct is_tuple<Tuple<Args...>> : std::true_type {};

template <typename Ty>
constexpr bool is_tuple_v = UTILS is_tuple<Ty>::value;

template <typename>
struct tuple_size;

template <typename... Args, template <typename...> typename Tuple>
struct tuple_size<Tuple<Args...>> {
    constexpr static std::size_t value = sizeof...(Args);
};

template <typename Tuple>
constexpr std::size_t tuple_size_v = tuple_size<std::remove_cvref_t<Tuple>>::value;

template <typename Ty, typename Type>
using cast_to_type = Type;

template <auto Candidate, typename... Args>
using is_nothrow_invocable_member_function =
    ::std::is_nothrow_invocable<decltype(std::mem_fn(Candidate)), Args...>;

template <auto Candidate, typename... Args>
constexpr bool is_nothrow_invocable_member_function_v =
    UTILS is_nothrow_invocable_member_function<Candidate, Args...>::value;

template <typename To, typename From>
struct same_volatile {
    using type = To;
};

template <typename To, typename From>
struct same_volatile<To, volatile From> {
    using type = volatile To;
};

template <typename To, typename From>
using same_volatile_t = typename same_volatile<To, From>::type;

template <typename To, typename From>
struct same_cv {
    using type = To;
};

template <typename To, typename From>
struct same_cv<To, const From> {
    using type = const To;
};

template <typename To, typename From>
struct same_cv<To, volatile From> {
    using type = volatile To;
};

template <typename To, typename From>
struct same_cv<To, const volatile From> {
    using type = const volatile To;
};

template <typename To, typename From>
using same_cv_t = typename same_cv<To, From>::type;

template <typename Ty, typename... Others>
struct first_of {
    using type = Ty;
};

template <typename... Args>
using first_of_t = typename first_of<Args...>::type;

} // namespace atom::utils
