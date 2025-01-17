#pragma once
#include <string_view>
#include "reflection.hpp"
#include "structures/tstring.hpp"

namespace atom::utils {

namespace internal {
constexpr std::size_t hash(std::string_view string) {
    // DJB2 Hash

    std::size_t value = 5381;
    for (const char c : string) {
        value = ((value << 5) + value) + c;
    }
    return value;
}
} // namespace internal

constexpr std::size_t hash(std::string_view name) {
    if (const auto found = name.find_last_of(' ');
        found != std::string_view::npos && found < name.length()) {
        return internal::hash(name.substr(found + 1));
    }
    else {
        return internal::hash(name);
    }
}

template <tstring_v Name>
consteval std::size_t hash() {
    return hash(Name.val);
}

template <typename Ty>
requires(!is_tstringv_v<Ty>)
std::size_t hash() {
    return hash(typeid(Ty).name());
}

} // namespace atom::utils
