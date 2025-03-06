#pragma once
#include <cstddef>
#include <cstring>
#include <ostream>
#include <string_view>

namespace atom::utils {

class basic_tstring {
public:
    constexpr basic_tstring(const char* string) : view(string) {}
    constexpr basic_tstring(const basic_tstring& obj) = default;
    constexpr basic_tstring(basic_tstring&& obj) noexcept : view(obj.view) {}
    constexpr basic_tstring& operator=(const basic_tstring&) = delete;
    constexpr basic_tstring& operator=(basic_tstring&&)      = delete;
    constexpr virtual ~basic_tstring()                       = default;

    [[nodiscard]] constexpr auto get() -> std::string_view& { return view; }
    [[nodiscard]] constexpr auto get() const -> const std::string_view& { return view; }

    [[nodiscard]] auto operator<=>(const basic_tstring& obj) const {
        return std::strcmp(view.data(), obj.view.data());
    }

    [[nodiscard]] auto operator==(const basic_tstring& obj) const -> bool {
        return view == obj.view;
    }

    [[nodiscard]] auto operator!=(const basic_tstring& obj) const -> bool {
        return view != obj.view;
    }

    std::ostream& operator<<(std::ostream& stream) {
        stream << view;
        return stream;
    }

private:
    std::string_view view;
};

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
template <std::size_t N>
struct tstring_v {
    constexpr tstring_v(const char (&arr)[N]) {
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

    char val[N]{};
};

template <typename>
struct is_tstringv : std::false_type {};

template <std::size_t N>
struct is_tstringv<tstring_v<N>> : std::true_type {};

template <typename Ty>
constexpr bool is_tstringv_v = is_tstringv<Ty>::value;

/**
 * @brief This type provides a way to obtain compile-time template string constants.
 *
 * @tparam String
 */
template <tstring_v String>
class tstring : public basic_tstring {
public:
    constexpr tstring() : basic_tstring(String.val) {}

    template <tstring_v Str>
    [[nodiscard]] constexpr auto operator<=>(const tstring<Str>& obj) const {
        return String <=> Str;
    }

    template <tstring_v Str>
    [[nodiscard]] constexpr auto operator==(const tstring<Str>& obj) const -> bool {
        return String == Str;
    }

    template <tstring_v Str>
    [[nodiscard]] constexpr auto operator!=(const tstring<Str>& obj) const -> bool {
        return String != Str;
    }

    std::ostream& operator<<(std::ostream& stream) const {
        stream << get();
        return stream;
    }
};

} // namespace atom::utils
