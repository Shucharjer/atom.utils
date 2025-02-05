#pragma once
#include "reflection/registery.hpp"

namespace atom::utils {

/**
 * @brief Register.
 *
 * Call the registry's registration function when creating an instance.
 * @tparam Ty The type want to register.
 * @tparam BasicConstexprExtend basic compile time extend information.
 * @tparam ConstexprExtend Advanced compile time extend information.
 */
template <typename Ty, typename BasicConstexprExtend, template <typename> typename ConstexprExtend>
struct type_register {
    type_register() { registry<BasicConstexprExtend, ConstexprExtend>::template enroll<Ty>(); }
    type_register(const type_register&)            = delete;
    type_register(type_register&&)                 = delete;
    type_register& operator=(const type_register&) = delete;
    type_register& operator=(type_register&&)      = delete;
    ~type_register() noexcept                      = default;
};

} // namespace atom::utils
