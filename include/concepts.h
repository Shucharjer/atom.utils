#pragma once
#include <concepts>
#include <type_traits>
#include "type_traits.h"

#define UCONCEPTS ::atom::utils::concepts::

namespace atom::utils::concepts {

template <typename Ty>
concept completed_type = requires { sizeof(Ty); };

template <typename Ty>
concept allocator = requires(Ty alloc) {
    typename Ty::value_type;
    typename Ty::size_type;
    {
        alloc.allocate(std::declval<typename Ty::size_type>())
    } -> ::std::same_as<typename Ty::value_type*>;
    alloc.deallocate(
        ::std::declval<typename Ty::value_type*>(), std::declval<typename Ty::size_type>()
    );
};

namespace internal {
template <typename>
struct rebind;

template <typename Ty, typename... Args, template <typename...> typename Allocator>
struct rebind<Allocator<Ty, Args...>> {
    template <typename RebindType>
    using type = Allocator<RebindType, Args...>;
};

template <typename Ty, typename Allocator>
using rebind_t = typename rebind<Allocator>::template type<Ty>;

template <typename Allocator, typename RebindType>
concept rebindable = requires { typename Allocator::template rebind_t<RebindType>; };
} // namespace internal

template <typename Ty>
concept rebindable_allocator = UCONCEPTS allocator<Ty> && requires() {
    UCONCEPTS internal::rebindable<Ty, int>;
    UCONCEPTS internal::rebindable<Ty, float>;
    UCONCEPTS internal::rebindable<Ty, double>;
};

namespace internal {

template <typename Shared, typename Type>
concept can_allocate = requires(Shared& pool, ::std::size_t count) {
    { pool->template allocate<Type>(count) } -> ::std::same_as<Type*>;
};

// clang-format off
template <typename Shared, typename Type>
concept can_deallocate = requires(Shared& pool, Type* ptr, std::size_t count) {
    pool->template deallocate<Type>(ptr, count);
};
// clang-format on

} // namespace internal

template <typename Ty>
concept memory_pool = requires(Ty pool, std::size_t count) {
    // shared_type.
    typename Ty::shared_type;

    // has function template allocate.
    requires UCONCEPTS internal::can_allocate<typename Ty::shared_type, int>;
    requires UCONCEPTS internal::can_allocate<typename Ty::shared_type, double>;
    requires UCONCEPTS internal::can_allocate<typename Ty::shared_type, char>;

    // has function template deallocate
    requires UCONCEPTS internal::can_deallocate<typename Ty::shared_type, int>;
    requires UCONCEPTS internal::can_deallocate<typename Ty::shared_type, double>;
    requires UCONCEPTS internal::can_deallocate<typename Ty::shared_type, char>;
};

template <typename Ty>
concept tuple = requires { is_tuple_v<Ty>; };

template <typename Ty>
concept pointer = requires { std::is_pointer_v<Ty>; };

template <typename Ty, typename Stream>
concept streamable = requires(Stream stream, const Ty& val) { stream << val; };

template <typename Ty, typename Stream>
concept extractable = requires(Stream stream, const Ty& val) { stream >> val; };

template <typename Ty>
concept type_with_value = requires { Ty::value_type; };

template <typename Rng>
concept common_range = ::std::ranges::range<Rng> &&
                       ::std::same_as<std::ranges::iterator_t<Rng>, ::std::ranges::sentinel_t<Rng>>;

template <typename Ty>
concept pure = std::is_same_v<Ty, std::remove_cvref_t<Ty>>;

} // namespace atom::utils::concepts
