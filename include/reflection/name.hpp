#pragma once
#include <string_view>

namespace atom::utils {

template <typename Ty>
struct nickname;

namespace internal {
template <typename Ty>
concept has_nickname = requires { nickname<Ty>::value; };
} // namespace internal

/**
 * @brief Get the name of a type.
 *
 */
template <typename Ty>
requires(!internal::has_nickname<Ty>)
[[nodiscard]] consteval inline std::string_view name_of() {
#ifdef _MSC_VER
    constexpr std::string_view funcname = __FUNCSIG__;
#else
    constexpr std::string_view funcname = __PRETTY_FUNCTION__;
#endif

#ifdef __clang__
    auto split = funcname.substr(0, funcname.size() - 1);
    return split.substr(split.find_last_of(' ') + 1);
#elif defined(__GNUC__)
    auto split = funcname.substr(77);
    return split.substr(0, split.size() - 50);
#elif defined(_MSC_VER)
    auto split = funcname.substr(110);
    split      = split.substr(split.find_first_of(' ') + 1);
    return split.substr(0, split.size() - 7);
#else
    static_assert(false, "Unsupportted compiler");
#endif
}

template <typename Ty>
requires internal::has_nickname<Ty>
[[nodiscard]] consteval inline std::string_view name_of() {
    return nickname<Ty>::value;
}

} // namespace atom::utils
