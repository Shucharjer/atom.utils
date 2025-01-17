#pragma once
#include <concepts>
#include "uconceptdef.hpp"

namespace atom::utils::concepts {

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

// concept: memory_pool
template <typename Ty>
concept mempool = requires(Ty pool, std::size_t count) {
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

} // namespace atom::utils::concepts
