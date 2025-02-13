#pragma once
#include <cstdint>
#include <type_traits>
#include "concepts/type.hpp"

namespace atom::utils {

using description_bits_base = std::uint32_t;

// clang-format off
enum class description_bits : description_bits_base {
    is_integral                        = 0b0000000000000000000000000000000000000000000000000000000000000001,
    is_floating_point                  = 0b0000000000000000000000000000000000000000000000000000000000000010,
    is_enum                            = 0b0000000000000000000000000000000000000000000000000000000000000100,
    is_union                           = 0b0000000000000000000000000000000000000000000000000000000000001000,
    is_class                           = 0b0000000000000000000000000000000000000000000000000000000000010000,
    is_object                          = 0b0000000000000000000000000000000000000000000000000000000000100000,
    is_trivial                         = 0b0000000000000000000000000000000000000000000000000000000001000000,
    is_standard_layout                 = 0b0000000000000000000000000000000000000000000000000000000010000000,
    is_empty                           = 0b0000000000000000000000000000000000000000000000000000000100000000,
    is_polymorphic                     = 0b0000000000000000000000000000000000000000000000000000001000000000,
    is_abstract                        = 0b0000000000000000000000000000000000000000000000000000010000000000,
    is_final                           = 0b0000000000000000000000000000000000000000000000000000100000000000,
    is_aggregate                       = 0b0000000000000000000000000000000000000000000000000001000000000000,
    is_function                        = 0b0000000000000000000000000000000000000000000000000010000000000000,
    is_default_constructible           = 0b0000000000000000000000000000000000000000000000000100000000000000,
    is_trivially_default_constructible = 0b0000000000000000000000000000000000000000000000001000000000000000,
    is_nothrow_default_constructible   = 0b0000000000000000000000000000000000000000000000010000000000000000,
    is_copy_constructible              = 0b0000000000000000000000000000000000000000000000100000000000000000,
    is_nothrow_copy_constructible      = 0b0000000000000000000000000000000000000000000001000000000000000000,
    is_trivially_copy_constructible    = 0b0000000000000000000000000000000000000000000010000000000000000000,
    is_move_constructible              = 0b0000000000000000000000000000000000000000000100000000000000000000,
    is_trivially_move_constructible    = 0b0000000000000000000000000000000000000000001000000000000000000000,
    is_nothrow_move_construcitble      = 0b0000000000000000000000000000000000000000010000000000000000000000,
    is_copy_assignable                 = 0b0000000000000000000000000000000000000000100000000000000000000000,
    is_trivially_copy_assignable       = 0b0000000000000000000000000000000000000001000000000000000000000000,
    is_nothrow_copy_assignable         = 0b0000000000000000000000000000000000000010000000000000000000000000,
    is_move_assignable                 = 0b0000000000000000000000000000000000000100000000000000000000000000,
    is_trivially_move_assignable       = 0b0000000000000000000000000000000000001000000000000000000000000000,
    is_nothrow_move_assignable         = 0b0000000000000000000000000000000000010000000000000000000000000000,
    is_destructible                    = 0b0000000000000000000000000000000000100000000000000000000000000000,
    is_trivially_destructible          = 0b0000000000000000000000000000000001000000000000000000000000000000,
    is_nothrow_destructible            = 0b0000000000000000000000000000000010000000000000000000000000000000
};
// clang-format on

template <concepts::pure Ty>
consteval static inline auto description_of() noexcept -> description_bits {
    using enum description_bits;
    description_bits_base mask = 0;
    mask |= std::is_integral_v<Ty> ? is_integral : 0;
    mask |= std::is_floating_point_v<Ty> ? is_floating_point : 0;
    mask |= std::is_enum_v<Ty> ? is_enum : 0;
    mask |= std::is_union_v<Ty> ? is_union : 0;
    mask |= std::is_class_v<Ty> ? is_class : 0;
    mask |= std::is_object_v<Ty> ? is_object : 0;
    mask |= std::is_trivial_v<Ty> ? is_trivial : 0;
    mask |= std::is_standard_layout_v<Ty> ? is_standard_layout : 0;
    mask |= std::is_empty_v<Ty> ? is_empty : 0;
    mask |= std::is_polymorphic_v<Ty> ? is_polymorphic : 0;
    mask |= std::is_abstract_v<Ty> ? is_abstract : 0;
    mask |= std::is_final_v<Ty> ? is_final : 0;
    mask |= std::is_aggregate_v<Ty> ? is_aggregate : 0;
    mask |= std::is_function_v<Ty> ? is_function : 0;
    mask |= std::is_default_constructible_v<Ty> ? is_default_constructible : 0;
    mask |= std::is_trivially_default_constructible_v<Ty> ? is_trivially_default_constructible : 0;
    mask |= std::is_nothrow_default_constructible_v<Ty> ? is_nothrow_default_constructible : 0;
    mask |= std::is_copy_constructible_v<Ty> ? is_copy_constructible : 0;
    mask |= std::is_trivially_copy_constructible_v<Ty> ? is_trivially_default_constructible : 0;
    mask |= std::is_nothrow_copy_constructible_v<Ty> ? is_nothrow_copy_constructible : 0;
    mask |= std::is_move_constructible_v<Ty> ? is_move_constructible : 0;
    mask |= std::is_trivially_move_constructible_v<Ty> ? is_trivially_move_constructible : 0;
    mask |= std::is_nothrow_move_constructible_v<Ty> ? is_nothrow_move_construcitble : 0;
    mask |= std::is_copy_assignable_v<Ty> ? is_copy_assignable : 0;
    mask |= std::is_trivially_copy_assignable_v<Ty> ? is_trivially_copy_assignable : 0;
    mask |= std::is_nothrow_copy_assignable_v<Ty> ? is_nothrow_copy_assignable : 0;
    mask |= std::is_move_constructible_v<Ty> ? is_move_constructible : 0;
    mask |= std::is_trivially_move_assignable_v<Ty> ? is_trivially_move_assignable : 0;
    mask |= std::is_nothrow_move_assignable_v<Ty> ? is_nothrow_move_assignable : 0;
    mask |= std::is_destructible_v<Ty> ? is_destructible : 0;
    mask |= std::is_trivially_destructible_v<Ty> ? is_trivially_destructible : 0;
    mask |= std::is_nothrow_destructible_v<Ty> ? is_nothrow_destructible : 0;
    return static_cast<description_bits>(mask);
}

template <concepts::pure Ty>
constexpr inline bool authenticity_of(const description_bits bits) noexcept {
    constexpr auto description = static_cast<description_bits_base>(description_of<Ty>());
    auto mask                  = static_cast<description_bits_base>(bits);
    auto result                = description & mask;
    return result == mask;
}

constexpr inline bool authenticity_of(
    const description_bits description, const description_bits bits
) noexcept {
    auto mask   = static_cast<description_bits_base>(bits);
    auto result = static_cast<description_bits_base>(description) & mask;
    return result == mask;
}

} // namespace atom::utils
