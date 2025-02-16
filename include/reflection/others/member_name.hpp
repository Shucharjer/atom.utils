#pragma once
#include <string_view>
#include <utility>
#include "concepts/type.hpp"
#include "reflection/member_count.hpp"
#include "structures/tstring.hpp"

namespace atom::utils {

template <concepts::has_field_traits Ty>
constexpr inline std::array<std::string_view, member_count_v<Ty>> member_names_of() noexcept {
    constexpr auto tuple = Ty::field_traits();
    std::array<std::string_view, member_count_v<Ty>> array;
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((array[Is] = std::get<Is>(tuple).name()), ...);
    }(std::make_index_sequence<member_count_v<Ty>>());
    return array;
}

/**
 * @brief Get the existance of a member.
 *
 */
template <tstring_v Name, concepts::has_field_traits Ty>
consteval inline auto existance_of() noexcept {
    constexpr auto names = member_names_of<Ty>();
    for (auto i = 0; i < member_count_v<Ty>; ++i) {
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
template <concepts::has_field_traits Ty>
constexpr inline auto existance_of(std::string_view name) noexcept -> size_t {
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
template <tstring_v Name, concepts::has_field_traits Ty>
consteval inline auto index_of() noexcept -> size_t {
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
template <concepts::has_field_traits Ty>
constexpr inline auto index_of(std::string_view name) noexcept -> size_t {
    constexpr auto count = member_count_v<Ty>;
    auto names           = member_names_of<Ty>();
    for (auto i = 0; i < count; ++i) {
        if (names[i] == name) {
            return i;
        }
    }
    return static_cast<std::size_t>(-1);
}

template <size_t Index, concepts::has_field_traits Ty>
consteval inline bool valid_index() noexcept {
    return Index < member_count_v<Ty>;
}

template <concepts::has_field_traits Ty>
constexpr inline bool valid_index(const size_t index) noexcept {
    return index < member_count_v<Ty>;
}

/**
 * @brief Get the name of a member.
 *
 */
template <size_t Index, concepts::has_field_traits Ty>
consteval inline auto name_of() noexcept {
    constexpr auto count = member_count_v<Ty>;
    static_assert(Index < count, "Index out of range");
    constexpr auto names = member_names_of<Ty>();
    return names[Index];
}

/**
 * @brief Get the name of a member.
 *
 */
template <concepts::has_field_traits Ty>
constexpr inline auto name_of(size_t index) {
    constexpr auto count = member_count_v<Ty>;
    assert(index < count); // Index out of range
    auto names = member_names_of<Ty>();
    return names[index];
}

} // namespace atom::utils
