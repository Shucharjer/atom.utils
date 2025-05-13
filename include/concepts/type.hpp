#pragma once
#include <concepts>
#include <tuple>
#include <type_traits>
#include <utility>
#include "core/type_traits.hpp"

namespace atom::utils::concepts {

// concept: completed
template <typename Ty>
concept completed = requires { sizeof(Ty); };

// concept: pure
template <typename Ty>
concept pure = std::is_same_v<Ty, std::remove_cvref_t<Ty>>;

// concept: pointer
template <typename Ty>
concept pointer = requires { std::is_pointer_v<std::remove_cvref_t<Ty>>; };

template <typename Ty>
concept optional = requires(Ty optional) {
    optional.value();
    optional.has_value();
    optional.operator*();
    typename std::remove_cvref_t<Ty>::value_type;
};

template <typename Ty>
concept aggregate = std::is_aggregate_v<std::remove_cvref_t<Ty>>;

template <typename Ty>
concept has_field_traits = requires() { std::remove_cvref_t<Ty>::field_traits(); };

template <typename Ty>
concept has_function_traits = requires() { std::remove_cvref_t<Ty>::function_traits(); };

template <typename Ty>
concept default_reflectible_aggregate = aggregate<Ty> && !has_field_traits<Ty>;

template <typename Ty>
concept reflectible = aggregate<Ty> || has_field_traits<Ty>;

template <typename Ty, typename T = Ty>
concept has_equal_operator = requires {
    { std::declval<Ty>() == std::declval<T>() } -> std::same_as<bool>;
};

template <typename Ty, typename T = Ty>
concept has_not_equal_operator = requires {
    { std::declval<Ty>() != std::declval<T>() } -> std::same_as<bool>;
};

template <typename Ty, typename T = Ty>
concept comparable = requires {
    requires has_equal_operator<Ty, T>;
    requires has_not_equal_operator<Ty, T>;
};

template <std::size_t Index, typename Ty>
concept std_gettible = requires(Ty& t) {
    { std::get<Index>(t) } -> std::same_as<std::tuple_element_t<Index, Ty>&>;
};

template <std::size_t Index, typename Ty>
concept member_gettible = requires(Ty& t) {
    { t.template get<Index>() } -> std::same_as<std::tuple_element_t<Index, Ty>&>;
};

template <std::size_t Index, typename Ty>
concept adl_gettible = requires(Ty& t) {
    { fake_copy_init(get<Index>(t)) } -> std::same_as<std::tuple_element_t<Index, Ty>>;
};

template <std::size_t Index, typename Ty>
concept gettible = std_gettible<Index, Ty> || member_gettible<Index, Ty> || adl_gettible<Index, Ty>;

template <typename Ty>
concept public_pair = requires(const std::remove_cv_t<std::remove_reference_t<Ty>>& val) {
    typename std::remove_cv_t<std::remove_reference_t<Ty>>::first_type;
    typename std::remove_cv_t<std::remove_reference_t<Ty>>::second_type;
    val.first;
    val.second;
};

template <typename Ty>
concept private_pair = requires(const std::remove_cv_t<std::remove_reference_t<Ty>>& val) {
    typename std::remove_cv_t<std::remove_reference_t<Ty>>::first_type;
    typename std::remove_cv_t<std::remove_reference_t<Ty>>::second_type;
    val.first();
    val.second();
};

template <typename Ty>
concept pair = public_pair<Ty> || private_pair<Ty>;

} // namespace atom::utils::concepts
