#pragma once
#include <concepts>
#include <functional>
#include <tuple>
#include <type_traits>

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
constexpr bool is_tuple_v = is_tuple<Ty>::value;

template <typename... Args>
constexpr auto tuple_size_v(const std::tuple<Args...>& tuple) noexcept {
    return sizeof...(Args);
}

template <typename Ty, typename Type>
using cast_to_type = Type;

template <auto Candidate, typename... Args>
using is_nothrow_invocable_member_function =
    std::is_nothrow_invocable<decltype(std::mem_fn(Candidate)), Args...>;

template <auto Candidate, typename... Args>
constexpr bool is_nothrow_invocable_member_function_v =
    is_nothrow_invocable_member_function<Candidate, Args...>::value;

} // namespace atom::utils
