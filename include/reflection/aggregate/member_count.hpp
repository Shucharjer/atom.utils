#pragma once
#include <concepts>
#include "concepts/type.hpp"

namespace atom::utils {

/**
 * @brief Universal type for deducing.
 *
 * If you define the operator, you may make things confusing!
 * like this: std::vector<int> vec(universal{});
 */
struct universal {
    template <typename Ty>
    operator Ty();
};

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

template <typename Ty, typename... Args>
constexpr inline auto member_count_of_impl() {
    if constexpr (std::constructible_from<Ty, Args..., universal>) {
        return member_count_of_impl<Ty, Args..., universal>();
    }
    else {
        return sizeof...(Args);
    }
}

} // namespace internal
/*! @endcond */

/**
 * @brief Get the member count of a type.
 *
 */
template <concepts::default_reflectible_aggregate Ty>
consteval inline auto member_count_of() {
    return internal::member_count_of_impl<std::remove_cvref_t<Ty>>();
}

} // namespace atom::utils
