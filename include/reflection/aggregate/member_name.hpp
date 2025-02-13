#pragma once
#include <array>
#include <type_traits>
#include <utility>
#include "concepts/type.hpp"
#include "reflection/aggregate/member_count.hpp"
#include "reflection/aggregate/tuple_view.hpp"
#include "structures/tstring.hpp"

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
template <concepts::aggregate Ty>
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

/**
 * @brief Get the existance of a member.
 *
 */
template <tstring_v Name, concepts::aggregate Ty>
consteval inline auto existance_of() {
    constexpr auto count = member_count_v<Ty>;
    constexpr auto names = member_names_of<Ty>();
    for (auto i = 0; i < count; ++i) {
        if (names[i] == Name.val) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Get the existance of a member.
 *
 */
template <concepts::aggregate Ty>
constexpr inline auto existance_of(std::string_view name) -> size_t {
    constexpr auto count = member_count_v<Ty>();
    auto names           = member_names_of<Ty>();
    for (auto i = 0; i < count; ++i) {
        if (names[i] == name) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Get the index of a member.
 *
 */
template <tstring_v Name, concepts::aggregate Ty>
consteval inline auto index_of() -> size_t {
    constexpr auto count = member_count_v<Ty>;
    constexpr auto names = member_names_of<Ty>();
    for (auto i = 0; i < count; ++i) {
        if (names[i] == Name.val) {
            return i;
        }
    }
    return static_cast<std::size_t>(-1);
}

/**
 * @brief Get the index of a member.
 *
 */
template <concepts::aggregate Ty>
constexpr inline auto index_of(std::string_view name) -> size_t {
    constexpr auto count = member_count_v<Ty>;
    auto names           = member_names_of<Ty>();
    for (auto i = 0; i < count; ++i) {
        if (names[i] == name) {
            return i;
        }
    }
    return static_cast<std::size_t>(-1);
}

template <size_t Index, concepts::aggregate Ty>
consteval inline bool valid_index() noexcept {
    return Index < member_count_v<Ty>;
}

template <concepts::aggregate Ty>
constexpr inline bool valid_index(const size_t index) noexcept {
    return index < member_count_v<Ty>;
}

/**
 * @brief Get the name of a member.
 *
 */
template <size_t Index, concepts::aggregate Ty>
consteval inline auto name_of() {
    constexpr auto count = member_count_v<Ty>;
    static_assert(Index < count, "Index out of range");
    constexpr auto names = member_names_of<Ty>();
    return names[Index];
}

/**
 * @brief Get the name of a member.
 *
 */
template <concepts::aggregate Ty>
constexpr inline auto name_of(size_t index) {
    return name_of<index, Ty>();
}

} // namespace atom::utils
