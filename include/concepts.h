#pragma once
#include <concepts>
#include <type_traits>
#include "type_traits.h"

namespace atom::utils::concepts {

template <typename Ty>
concept completed_type = requires { sizeof(Ty); };

namespace internal {

template <typename Shared, typename Type>
concept can_allocate = requires(Shared& pool, std::size_t count) {
    { pool->template allocate<Type>(count) } -> std::same_as<Type*>;
};

// clang-format off
template <typename Shared, typename Type>
concept can_deallocate = requires(Shared& pool, Type* ptr, std::size_t count) {
    pool->template deallocate<Type>(ptr, count);
};
// clang-format on

} // namespace

template <typename Ty>
concept memory_pool = requires(Ty pool, std::size_t count) {
    // shared_type.
    typename Ty::shared_type;

    // has function template allocate.
    requires internal::can_allocate<typename Ty::shared_type, int>;
    requires internal::can_allocate<typename Ty::shared_type, double>;
    requires internal::can_allocate<typename Ty::shared_type, char>;

    // has function template deallocate
    requires internal::can_deallocate<typename Ty::shared_type, int>;
    requires internal::can_deallocate<typename Ty::shared_type, double>;
    requires internal::can_deallocate<typename Ty::shared_type, char>;
};

template <typename Ty>
concept tuple = requires { is_tuple<Ty>::value; };

template <typename Ty>
concept pointer = requires { std::is_pointer_v<Ty>; };

template <typename Ty, typename Stream>
concept streamable = requires(Stream stream, const Ty& val) { stream << val; };

template <typename Ty, typename Stream>
concept extractable = requires(Stream stream, const Ty& val) { stream >> val; };

template <typename Ty>
concept type_with_value = requires { Ty::value_type; };

template <typename Rng>
concept common_range = std::ranges::range<Rng> &&
                       std::same_as<std::ranges::iterator_t<Rng>, std::ranges::sentinel_t<Rng>>;

} // namespace atom::utils::concepts
