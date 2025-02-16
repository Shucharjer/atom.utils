#pragma once
#include <concepts>
#include "uconceptdef.hpp"

namespace atom::utils::concepts {

/*! @cond TURN_OFF_DOXYGEN*/
namespace internal {

template <typename Shared, typename Type>
concept can_allocate = requires(Shared& pool, ::std::size_t count) {
    { pool->template allocate<Type>(count) } -> ::std::same_as<Type*>;
};

template <typename Shared, typename Type>
concept can_deallocate = requires(Shared& pool, Type* ptr, std::size_t count) {
    pool->template deallocate<Type>(ptr, count);
};

} // namespace internal
/*! @endcond */

// concept: memory_pool
/**
 * @brief Memory pool
 *
 * A memory pool can allocate memory blocks with different sizes.
 * And for the security, we requires it has its definition of `shared_type`, like `shared_ptr` or
 * something.
 */
template <typename Ty>
concept mempool = requires(Ty pool, std::size_t count) {
    // shared_type.
    typename std::remove_cvref_t<Ty>::shared_type;

    // has function template allocate.
    requires UCONCEPTS internal::can_allocate<typename std::remove_cvref_t<Ty>::shared_type, int>;
    requires UCONCEPTS
        internal::can_allocate<typename std::remove_cvref_t<Ty>::shared_type, double>;
    requires UCONCEPTS internal::can_allocate<typename std::remove_cvref_t<Ty>::shared_type, char>;

    // has function template deallocate
    requires UCONCEPTS internal::can_deallocate<typename std::remove_cvref_t<Ty>::shared_type, int>;
    requires UCONCEPTS
        internal::can_deallocate<typename std::remove_cvref_t<Ty>::shared_type, double>;
    requires UCONCEPTS
        internal::can_deallocate<typename std::remove_cvref_t<Ty>::shared_type, char>;
};

} // namespace atom::utils::concepts
