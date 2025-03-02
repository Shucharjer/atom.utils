#pragma once
#include <memory>
#include "core/langdef.hpp"

namespace atom::utils {
template <typename Ty>
struct default_destroyer;

namespace internal {

template <typename Ty>
ATOM_FORCE_INLINE constexpr void destroy(Ty* ptr) noexcept(
    noexcept(std::destroy_at(ptr))) {
    std::destroy_at(ptr);
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
