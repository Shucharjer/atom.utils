#pragma once
#include "concepts/type.hpp"

namespace atom::utils {

/**
 * @brief Base of class template field_traits.
 *
 */
struct basic_field_traits;

/**
 * @brief Traits of filed.
 */
template <typename = void>
struct field_traits;

/**
 * @brief Base of class template function_traits.
 *
 */
struct basic_function_traits;

/**
 * @brief Traits of function.
 */
template <typename>
struct function_traits;

/**
 * @class basic_constexpr_extend
 * @brief Extend informations about a type, which could be determined at compile time.
 *
 */
struct basic_constexpr_extend;

/**
 * @brief Extend informations about a type, which could be determined at compile time.
 */
template <::atom::utils::concepts::pure>
struct constexpr_extend;

/**
 * @class extend
 * @brief Extend informations about a type, which could only be determined at runtime.
 *
 */
struct extend;

/**
 * @brief Base of class template `reflected`.
 *
 * This could provide a same base to collect reflected informations to a contain easily.
 */
template <::atom::utils::concepts::pure BaseConstexprExtend = basic_constexpr_extend>
struct basic_reflected;

/**
 * @brief Reflected informations of a type.
 *
 * @tparam Ty Reflected type.
 */
template <
    ::atom::utils::concepts::pure Ty,
    ::atom::utils::concepts::pure BasicConstexprExtend                = basic_constexpr_extend,
    template <::atom::utils::concepts::pure> typename ConstexprExtend = constexpr_extend>
struct reflected;

template <typename BasicConstexprExtend, template <typename> typename ConstexprExtend>
class registry;

using basic_registry = registry<basic_constexpr_extend, constexpr_extend>;
} // namespace atom::utils
