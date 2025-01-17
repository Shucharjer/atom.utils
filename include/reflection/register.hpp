#pragma once
#include "reflection/registery.hpp"

namespace atom::utils {

template <typename Ty, typename BasicConstexprExtend, template <typename> typename ConstexprExtend>
struct type_register {
    type_register() { registry<BasicConstexprExtend, ConstexprExtend>::template enroll<Ty>(); }
    ~type_register() noexcept = default;
};

} // namespace atom::utils
