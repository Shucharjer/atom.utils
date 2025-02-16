#pragma once
#include "concepts/type.hpp"

namespace atom::utils {

template <concepts::has_field_traits Ty>
constexpr inline size_t member_count_of() noexcept {
    return std::tuple_size_v<decltype(Ty::field_traits())>;
}

} // namespace atom::utils
