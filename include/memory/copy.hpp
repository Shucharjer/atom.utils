#pragma once
#include <algorithm>
#include <cstring>
#include <type_traits>
#include "core/langdef.hpp"

namespace atom::utils {

/**
 * @brief Memcpy on runtime, used to assign to trivially assignable type.
 *
 * @param[out] dst Destination.
 * @param[in] src Source.
 */
template <typename Ty, typename T = Ty>
requires std::is_trivially_assignable_v<Ty, T>
constexpr void rtmemcpy(Ty& dst, T&& src) noexcept {
    if (std::is_constant_evaluated()) {
        dst = std::forward<T>(src);
    }
    else {
        if constexpr (sizeof(Ty) > num_sixteen) {
            std::memcpy(std::addressof(dst), std::addressof(src), sizeof(Ty));
        }
        else {
            dst = std::forward<T>(src);
        }
    }
}

} // namespace atom::utils
