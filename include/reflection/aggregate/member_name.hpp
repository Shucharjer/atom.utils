#pragma once
#include <array>
#include <string_view>
#include <type_traits>
#include <utility>
#include "concepts/type.hpp"
#include "reflection/aggregate/tuple_view.hpp"
#include "reflection/member_count.hpp"

namespace atom::utils {

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

template <auto Ptr> // name
consteval inline std::string_view member_name_of() {
#ifdef _MSC_VER
    constexpr std::string_view funcname = __FUNCSIG__;
#else
    constexpr std::string_view funcname = __PRETTY_FUNCTION__;
#endif

#ifdef __clang__
    auto split = funcname.substr(0, funcname.size() - 2);
    return split.substr(split.find_last_of(":.") + 1);
#elif defined(__GNUC__)
    auto split = funcname.substr(0, funcname.rfind(")}"));
    return split.substr(split.find_last_of(":") + 1);
#elif defined(_MSC_VER)
    auto split = funcname.substr(0, funcname.rfind("}>"));
    return split.substr(split.rfind("->") + 2);
#else
    static_assert(false, "Unsupportted compiler");
#endif
}

template <typename Ty>
struct wrapper {
    Ty val;
};

template <typename Ty>
wrapper(Ty) -> wrapper<Ty>;

template <typename Ty>
constexpr inline auto wrap(const Ty& arg) noexcept {
    return wrapper{ arg };
}

} // namespace internal
/*! @endcond */

/**
 * @brief Get members' name of a type.
 *
 */
template <concepts::default_reflectible_aggregate Ty>
constexpr inline std::array<std::string_view, member_count_v<Ty>> member_names_of() {
    constexpr size_t count = member_count_v<Ty>;
    using pure             = std::remove_cvref_t<Ty>;
    std::array<std::string_view, count> array;
    constexpr auto tuple = internal::struct_to_tuple_view<Ty>();
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((array[Is] = internal::member_name_of<internal::wrap(std::get<Is>(tuple))>()), ...);
    }(std::make_index_sequence<count>());
    return array;
}

} // namespace atom::utils
