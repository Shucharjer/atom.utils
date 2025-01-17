#pragma once
#include <type_traits>
#include "core/type_traits.hpp"
#include "uconceptdef.hpp"

namespace atom::utils::concepts {

// concept: completed
template <typename Ty>
concept completed = requires { sizeof(Ty); };

// concept: pure
template <typename Ty>
concept pure = std::is_same_v<Ty, std::remove_cvref_t<Ty>>;

// concept: pointer
template <typename Ty>
concept pointer = requires { std::is_pointer_v<Ty>; };

// concept: tuple
template <typename Ty>
concept tuple = ::atom::utils::is_tuple_v<Ty>;

} // namespace atom::utils::concepts
