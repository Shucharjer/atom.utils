#pragma once
#include "concepts/type.hpp"
#include "reflection/aggregate/member_name.hpp"
#include "reflection/others/member_name.hpp"

namespace atom::utils {

/**
 * @brief Get the existance of a member.
 *
 */
template <tstring_v Name, concepts::reflectible Ty>
[[nodiscard]] consteval inline auto existance_of() {
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
template <concepts::reflectible Ty>
[[nodiscard]] constexpr inline auto existance_of(std::string_view name) -> size_t {
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
template <tstring_v Name, concepts::reflectible Ty>
[[nodiscard]] consteval inline auto index_of() -> size_t {
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
template <concepts::reflectible Ty>
[[nodiscard]] constexpr inline auto index_of(std::string_view name) -> size_t {
    constexpr auto count = member_count_v<Ty>;
    auto names           = member_names_of<Ty>();
    for (auto i = 0; i < count; ++i) {
        if (names[i] == name) {
            return i;
        }
    }
    return static_cast<std::size_t>(-1);
}

template <size_t Index, concepts::reflectible Ty>
[[nodiscard]] consteval inline bool valid_index() noexcept {
    return Index < member_count_v<Ty>;
}

template <concepts::reflectible Ty>
[[nodiscard]] constexpr inline bool valid_index(const size_t index) noexcept {
    return index < member_count_v<Ty>;
}

/**
 * @brief Get the name of a member.
 *
 */
template <size_t Index, concepts::reflectible Ty>
[[nodiscard]] consteval inline auto name_of() {
    constexpr auto count = member_count_v<Ty>;
    static_assert(Index < count, "Index out of range");
    constexpr auto names = member_names_of<Ty>();
    return names[Index];
}

/**
 * @brief Get the name of a member.
 *
 */
template <concepts::reflectible Ty>
[[nodiscard]] constexpr inline auto name_of(size_t index) {
    return name_of<index, Ty>();
}

} // namespace atom::utils
