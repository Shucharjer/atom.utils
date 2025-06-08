#pragma once
#include <cstddef>
#include <cstring>
#include <ostream>

namespace atom::utils {

/**
 * @brief String that could be used as template argument
 *
 * @tparam N Size of string's length. It could be decl by compiler.
 *
 * Example:
 *
 * Declare a function template like `template <tstring String>
 * void foo();`, then use it like `foo<"example">();`
 */
template <std::size_t N, typename Char = char>
struct tstring_v {
    constexpr tstring_v(const Char (&arr)[N]) {
        // memcpy is not a compile-time function
        // std::memcpy(val, arr, N);
        std::copy(arr, arr + N, val);
    }

    template <std::size_t Num>
    constexpr auto operator<=>(const tstring_v<Num>& obj) const {
        return std::strcmp(val, obj.val);
    }

    template <std::size_t Num>
    constexpr auto operator==(const tstring_v<Num>& obj) const -> bool {
        return !(*this <=> obj);
    }

    template <std::size_t Num>
    constexpr auto operator!=(const tstring_v<Num>& obj) const -> bool {
        return (*this <=> obj);
    }

    std::ostream& operator<<(std::ostream& stream) {
        stream << val;
        return stream;
    }

    Char val[N];
};

template <typename>
struct is_tstringv : std::false_type {};

template <std::size_t N>
struct is_tstringv<tstring_v<N>> : std::true_type {};

template <typename Ty>
constexpr bool is_tstringv_v = is_tstringv<Ty>::value;

} // namespace atom::utils
