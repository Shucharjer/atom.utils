#pragma once
#include <concepts>

namespace atom::utils::concepts {

// concept: allocator

/**
 * @brief Normal allocator.
 * It could allocate a space to store an instance with type of 'Ty'.
 *
 * @tparam Ty
 */
template <typename Ty>
concept allocator = requires(Ty& alloc) {
    typename std::remove_cvref_t<Ty>::value_type;
    typename std::remove_cvref_t<Ty>::size_type;
    {
        alloc.allocate(std::declval<typename std::remove_cvref_t<Ty>::size_type>())
    } -> ::std::same_as<typename std::remove_cvref_t<Ty>::value_type*>;
    alloc.deallocate(
        ::std::declval<typename std::remove_cvref_t<Ty>::value_type*>(),
        std::declval<typename std::remove_cvref_t<Ty>::size_type>());
};

/*! @cond TURN_OFF_DOXYGEN */
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
/*! @endcond */

// concept: rebindable_allocator
/**
 * @brief Allocators could be initialized with allocators of other types.
 *
 */
template <typename Ty>
concept rebindable_allocator = ::atom::utils::concepts::allocator<Ty> && requires() {
    requires internal::rebindable<Ty, int>;
    requires internal::rebindable<Ty, float>;
    requires internal::rebindable<Ty, double>;
};

} // namespace atom::utils::concepts
