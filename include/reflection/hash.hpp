#pragma once
#include <regex>
#include <string>
#include <string_view>
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

inline std::string process_name(const std::string& str) {
    std::string result = str;
    std::regex prefix_pattern(R"(class\ |struct\ |enum\ )");
    result = std::regex_replace(result, prefix_pattern, "");
    std::regex comma_pattern(R"(,\ |\ ,)");
    result = std::regex_replace(result, comma_pattern, ",");
    std::regex endtparam_pattern(R"(> |\ >)");
    result = std::regex_replace(result, endtparam_pattern, ">");
    std::regex pattern(R"(:\ |\ :)");
    result = std::regex_replace(result, pattern, ":");
    result = std::regex_replace(result, pattern, ":");

    return result;
}

inline bool valid_name(const std::string& str) {
    std::regex pattern(R"(class\ |struct\ |enum\ |,\ |\ ,|>\ |\ >|:\ |\ :)");
    return !std::regex_match(str, pattern);
}

} // namespace internal
/*! @endcond */

/**
 * @brief Get the hash value of a string view.
 *
 * @return constexpr std::size_t Hash value.
 */
constexpr std::size_t hash(std::string_view str) {
    if (const auto found = str.find_last_of(' ');
        found != std::string_view::npos && found < str.length()) {
        return internal::hash(str.substr(found + 1));
    }
    else {
        return internal::hash(str);
    }
}

/**
 * @brief Get the hash value of a template string.
 *
 * @tparam Str Template string.
 * @return consteval std::size_t Hash value.
 */
template <tstring_v Str>
consteval std::size_t hash() {
    return hash(Str.val);
}

/**
 * @brief Get the hash value of a type.
 *
 * @tparam Ty The type.
 */
template <typename Ty>
requires(!is_tstringv_v<Ty>)
std::size_t hash() {
    return hash(internal::process_name(typeid(Ty).name()));
}

} // namespace atom::utils
