#pragma once
#include <string_view>
#include "reflection/name.hpp"
#include "structures/tstring.hpp"

namespace atom::utils {

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
constexpr inline std::size_t hash(std::string_view string) {
    // DJB2 Hash

    const size_t magic_initial_value = 5381;
    const size_t magic               = 5;

    std::size_t value = magic_initial_value;
    for (const char c : string) {
        value = ((value << magic) + value) + c;
    }
    return value;
}

} // namespace internal
/*! @endcond */

template <tstring_v Str>
consteval inline size_t hash_of() {
    return internal::hash(Str.val);
}

template <typename Ty>
constexpr inline size_t hash_of() {
    constexpr auto name = name_of<Ty>();
    return internal::hash(name);
}

inline size_t hash_of(std::string_view str) { return internal::hash(str); }

} // namespace atom::utils
