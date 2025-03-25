#pragma once
#include "concepts/type.hpp"

namespace atom::utils {

template <std::size_t Index, typename Ty>
requires concepts::gettible<Index, Ty>
constexpr inline decltype(auto) uniget(Ty& inst) noexcept {
    using namespace concepts;
    if constexpr (std_gettible<Index, Ty>) {
        return std::get<Index>(inst);
    }
    else if constexpr (member_gettible<Index, Ty>) {
        return inst.template get<Index>();
    }
    else if constexpr (adl_gettible<Index, Ty>) {
        return get<Index>(inst);
    }
}

} // namespace atom::utils
