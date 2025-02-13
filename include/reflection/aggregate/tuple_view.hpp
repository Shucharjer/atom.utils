#pragma once
#include <type_traits>
#include "reflection/aggregate/tuple_view_helper.hpp"

/*! @cond TURN_OFF_DOXYGEN */
namespace atom::utils::internal {

/**
 * @brief Get the member type pointers.
 *
 */
template <typename Ty>
constexpr inline auto struct_to_tuple_view() {
    return internal::tuple_view_helper<std::remove_cvref_t<Ty>>::tuple_view();
}

template <typename Ty>
constexpr inline auto object_to_tuple_view(const Ty& obj) {
    return internal::tuple_view_helper<std::remove_cvref_t<Ty>>::tuple_view(obj);
}

template <typename Ty>
constexpr inline auto object_to_tuple_view(Ty& obj) {
    return internal::tuple_view_helper<std::remove_cvref_t<Ty>>::tuple_view(obj);
}

} // namespace atom::utils::internal
/*! @endcond */
