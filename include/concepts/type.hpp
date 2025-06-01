#pragma once
#include <concepts>
#include <type_traits>
#include <utility>

namespace atom::utils::concepts {

// concept: completed
template <typename Ty>
concept completed = requires { sizeof(Ty) >= 1; };

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

} // namespace atom::utils::concepts
