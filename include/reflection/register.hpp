#pragma once
#include "reflection/registry.hpp"

namespace atom::utils {

/**
 * @brief Register.
 *
 * Call the registry's registration function when creating an instance.
 * @tparam Ty The type want to register.
 * @tparam Placeholder The registry would register in.
 */
template <typename Ty, typename Placeholder>
struct type_register {
    type_register() { registry<Placeholder>::template enroll<Ty>(); }
    type_register(const type_register&)            = delete;
    type_register(type_register&&)                 = delete;
    type_register& operator=(const type_register&) = delete;
    type_register& operator=(type_register&&)      = delete;
    ~type_register() noexcept                      = default;
};

} // namespace atom::utils
