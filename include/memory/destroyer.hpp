#pragma once
#include <ranges>
#include <type_traits>
#include "core/langdef.hpp"

namespace atom::utils {
template <typename Ty>
struct default_destroyer;

namespace internal {

template <typename Begin, typename End>
ATOM_FORCE_INLINE constexpr void destroy_range(Begin begin, End end) noexcept(
    std::is_nothrow_destructible_v<std::iter_value_t<Begin>>) {
    using value_type = std::iter_value_t<Begin>;
    if constexpr (!std::is_trivially_destructible_v<value_type>) {
        for (; begin != end; ++begin) {
            (*begin)->~value_type();
        }
    }
}

template <typename Ty>
ATOM_FORCE_INLINE constexpr void destroy(Ty* ptr) noexcept(std::is_nothrow_destructible_v<Ty>) {
    if constexpr (!std::is_destructible_v<Ty>) {
        return;
    }

    if constexpr (std::is_array_v<Ty>) {
        destroy_range(std::begin(*ptr), std::end(*ptr));
    }
    else if constexpr (!std::is_trivially_destructible_v<Ty>) {
        ptr->~Ty();
    }
}

template <typename Ty>
constexpr void wrapped_destroy(void* ptr) noexcept(noexcept(destroy(static_cast<Ty*>(ptr)))) {
    destroy(static_cast<Ty*>(ptr));
}

} // namespace internal

template <typename Ty>
struct default_destroyer {
    using value_type = Ty;

    template <typename T>
    using rebind_t = default_destroyer<T>;

    inline constexpr void operator()(Ty* const ptr) const noexcept { internal::destroy(ptr); }
};

} // namespace atom::utils
