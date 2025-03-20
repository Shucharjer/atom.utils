#pragma once
#include <cstring>
#include <type_traits>

#if __has_include(<core/langdef.hpp>)
    #include "core/langdef.hpp"
#else
constexpr auto num_sixteen = 16;
#endif

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
    if (std::is_constant_evaluated()) [[unlikely]] {
        dst = std::forward<T>(src);
    }
    else [[likely]] {
        if constexpr (sizeof(Ty) > num_sixteen) {
            std::memcpy(std::addressof(dst), std::addressof(src), sizeof(Ty));
        }
        else {
            dst = std::forward<T>(src);
        }
    }
}

} // namespace atom::utils
