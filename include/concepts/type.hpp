#pragma once
#include <type_traits>

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

} // namespace atom::utils::concepts
