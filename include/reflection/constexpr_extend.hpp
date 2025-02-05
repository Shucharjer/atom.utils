#pragma once
#include "concepts/type.hpp"
#include "core.hpp"
#include "reflection.hpp"

namespace atom::utils {

struct basic_constexpr_extend {

    /**
     * @class constexpr_extend_info Information could be got in compile time.
     * @brief Providing a structure to using designated list initialization, which could omit
     * some operations when defining extended information of the type.
     *
     */
    struct constexpr_extend_info {
        bool is_default_constructible = true;
        bool is_trivial_v             = true;
        bool is_copy_constructible    = true;
        bool is_move_constructible    = true;
        bool is_copy_assignable       = true;
        bool is_move_assignable       = true;
        bool is_destructible          = true;
        bool is_aggregate_v           = false;
        bool is_enum                  = false;
    };

    constexpr_extend_info info;

    constexpr basic_constexpr_extend()                                         = default;
    constexpr basic_constexpr_extend(const basic_constexpr_extend&)            = default;
    constexpr basic_constexpr_extend(basic_constexpr_extend&&)                 = default;
    constexpr basic_constexpr_extend& operator=(const basic_constexpr_extend&) = default;
    constexpr basic_constexpr_extend& operator=(basic_constexpr_extend&&)      = default;
    constexpr virtual ~basic_constexpr_extend()                                = default;

    constexpr basic_constexpr_extend(const constexpr_extend_info& info) : info(info) {}
};

template <::atom::utils::concepts::pure Ty>
struct constexpr_extend : public ::atom::utils::basic_constexpr_extend {
    constexpr constexpr_extend()
        : ::atom::utils::basic_constexpr_extend{
              { .is_default_constructible = std::is_default_constructible_v<Ty>,
               .is_trivial_v             = std::is_trivial_v<Ty>,
               .is_copy_constructible    = std::is_copy_constructible_v<Ty>,
               .is_move_constructible    = std::is_move_constructible_v<Ty>,
               .is_copy_assignable       = std::is_copy_assignable_v<Ty>,
               .is_move_assignable       = std::is_move_assignable_v<Ty>,
               .is_destructible          = std::is_destructible_v<Ty>,
               .is_aggregate_v           = std::is_aggregate_v<Ty>,
               .is_enum                  = std::is_enum_v<Ty> }
    } {}
    constexpr constexpr_extend(const constexpr_extend&)            = default;
    constexpr constexpr_extend(constexpr_extend&&)                 = default;
    constexpr constexpr_extend& operator=(const constexpr_extend&) = default;
    constexpr constexpr_extend& operator=(constexpr_extend&&)      = default;
    constexpr ~constexpr_extend() override                         = default;
};

} // namespace atom::utils
