/**
 * @file reflection.hpp
 * @brief A one header file reflection framework.
 * @date 2025-02-21
 *
 */
#pragma once
#include <array>
#include <string_view>
#include <tuple>

#if __has_include("core/langdef.hpp")
    #include "core/langdef.hpp"
    #ifdef ATOM_FORCE_INLINE
        #define FORCE_INLINE ATOM_FORCE_INLINE
    #endif
#endif

#ifndef FORCE_INLINE
    #ifdef _MSC_VER
        #define FORCE_INLINE __forceinline
    #elif defined(__GNUC__) || defined(__clang__)
        #define FORCE_INLINE inline __attribute__((__always_inline__))
    #else
        #define FORCE_INLINE inline
    #endif
#endif

#if __has_include(<concepts/type.hpp>)
    #include <concepts/type.hpp>
#else
namespace atom::utils::concepts {
template <typename Ty>
concept aggregate = std::is_aggregate_v<std::remove_cvref_t<Ty>>;

template <typename Ty>
concept has_field_traits = requires() { std::remove_cvref_t<Ty>::field_traits(); };

template <typename Ty>
concept has_function_traits = requires() { std::remove_cvref_t<Ty>::function_traits(); };

template <typename Ty>
concept default_reflectible_aggregate = aggregate<Ty> && !has_field_traits<Ty>;

template <typename Ty>
concept reflectible = aggregate<Ty> || has_field_traits<Ty>;
} // namespace atom::utils::concepts
#endif

#if __has_include(<core/type.hpp>)
    #include <core/type.hpp>
#else
namespace atom::utils {
/**
 * @brief Universal type for deducing.
 *
 * If you define the operator, you may make things confusing!
 * like this: std::vector<int> vec(universal{});
 */
struct universal {
    template <typename Ty>
    operator Ty();
};
} // namespace atom::utils
#endif

#if __has_include(<structures/tstring.hpp>)
    #include <structures/tstring.hpp>
#else
namespace atom::utils {
template <std::size_t N>
struct tstring_v {
    constexpr tstring_v(const char (&arr)[N]) { std::copy(arr, arr + N, val); }

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
} // namespace atom::utils
#endif

#if __has_include(<memory/destroyer.hpp>)
    #include <memory/destroyer.hpp>
#else
/*! @cond TURN_OFF_DOXYGEN */
namespace atom::utils::internal {

template <typename Begin, typename End>
FORCE_INLINE constexpr void destroy_range(Begin begin, End end) noexcept(
    std::is_nothrow_destructible_v<std::iter_value_t<Begin>>) {
    using value_type = std::iter_value_t<Begin>;
    if constexpr (!std::is_trivially_destructible_v<value_type>) {
        for (; begin != end; ++begin) {
            (*begin)->~value_type();
        }
    }
}

template <typename Ty>
FORCE_INLINE constexpr void destroy(Ty* ptr) noexcept(std::is_nothrow_destructible_v<Ty>) {
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

} // namespace atom::utils::internal
/*! @endcond */
#endif

///////////////////////////////////////////////////////////////////////////////
// module: field_traits & function_traits
///////////////////////////////////////////////////////////////////////////////

namespace atom::utils {
/**
 * @brief Base of class template field_traits.
 *
 */
struct basic_field_traits;

/**
 * @brief Traits of filed.
 */
template <typename = void>
struct field_traits;

struct basic_field_traits {
    explicit constexpr basic_field_traits(const char* name) : name_(name) {}
    explicit constexpr basic_field_traits(std::string_view name) : name_(name) {}

    constexpr basic_field_traits(const basic_field_traits&)            = default;
    constexpr basic_field_traits& operator=(const basic_field_traits&) = default;

    constexpr basic_field_traits(basic_field_traits&& obj) noexcept : name_(obj.name_) {}
    constexpr basic_field_traits& operator=(basic_field_traits&& obj) noexcept {
        if (this != &obj) {
            name_ = obj.name_;
        }
        return *this;
    }

    constexpr ~basic_field_traits() = default;

    [[nodiscard]] constexpr auto name() const -> std::string_view { return name_; }

private:
    std::string_view name_;
};

template <>
struct field_traits<void> : public ::atom::utils::basic_field_traits {
    using type = void;

    explicit constexpr field_traits() : basic_field_traits("void") {}
};

template <typename Ty>
requires(!std::is_void_v<Ty>)
struct field_traits<Ty*> : public ::atom::utils::basic_field_traits {
    using type = Ty;

    explicit constexpr field_traits(const char* name, Ty* pointer)
        : ::atom::utils::basic_field_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr auto get() -> Ty& { return *pointer_; }
    [[nodiscard]] constexpr auto get() const -> const Ty& { return *pointer_; }

    [[nodiscard]] constexpr auto pointer() const noexcept -> Ty* { return pointer_; }

private:
    Ty* pointer_;
};

template <typename Ty, typename Class>
struct field_traits<Ty Class::*> : public ::atom::utils::basic_field_traits {
    using type = Ty;

    explicit constexpr field_traits(const char* name, Ty Class::* pointer)
        : ::atom::utils::basic_field_traits(name), pointer_(pointer) {
        // set_name(name_);
    }

    [[nodiscard]] constexpr auto get(Class& instance) const -> Ty& { return instance.*pointer_; }
    [[nodiscard]] constexpr auto get(const Class& instance) const -> const Ty& {
        return instance.*pointer_;
    }

    [[nodiscard]] constexpr auto pointer() const noexcept -> Ty Class::* { return pointer_; }

private:
    Ty Class::* pointer_;
};

/**
 * @brief Base of class template function_traits.
 *
 */
struct basic_function_traits;

/**
 * @brief Traits of function.
 */
template <typename>
struct function_traits;

struct basic_function_traits {
    explicit constexpr basic_function_traits(const char* name) : name_(name) {}
    explicit constexpr basic_function_traits(std::string_view name) : name_(name) {}

    constexpr basic_function_traits(const basic_function_traits&)            = default;
    constexpr basic_function_traits& operator=(const basic_function_traits&) = default;

    constexpr basic_function_traits(basic_function_traits&& obj) noexcept : name_(obj.name_) {}
    constexpr basic_function_traits& operator=(basic_function_traits&& obj) noexcept {
        if (this != &obj) {
            name_ = obj.name_;
        }
        return *this;
    }

    constexpr virtual ~basic_function_traits() = default;

    [[nodiscard]] constexpr auto name() const -> std::string_view { return name_; }

private:
    std::string_view name_;
};

template <typename Ret, typename... Args>
struct function_traits<Ret (*)(Args...)> : public ::atom::utils::basic_function_traits {
    using func_type = Ret(Args...);

    explicit constexpr function_traits(const char* name, Ret (*pointer)(Args...))
        : ::atom::utils::basic_function_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr std::size_t num_args() const { return sizeof...(Args); }

    [[nodiscard]] constexpr auto call(Args&&... args) const -> Ret {
        return pointer_(std::forward<Args>(args)...);
    }

    [[nodiscard]] constexpr auto pointer() const noexcept -> Ret (*)(Args...) { return pointer_; }

private:
    Ret (*pointer_)(Args...);
};

template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret (Class::*)(Args...)> : public ::atom::utils::basic_function_traits {
    using func_type = Ret(Args...);

    explicit constexpr function_traits(const char* name, Ret (Class::*pointer)(Args...))
        : ::atom::utils::basic_function_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr std::size_t num_args() const { return sizeof...(Args); }

    [[nodiscard]] constexpr auto call(Class& instance, Args&&... args) const -> Ret {
        (instance.*pointer_)(std::forward<Args>(args)...);
    }

    [[nodiscard]] constexpr auto pointer() const noexcept -> Ret (Class::*)(Args...) {
        return pointer_;
    }

private:
    Ret (Class::*pointer_)(Args...);
};
} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// module: name_of <typename>
///////////////////////////////////////////////////////////////////////////////

namespace atom::utils {
/**
 * @brief Get the name of a type.
 *
 */
template <typename Ty>
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
struct alias_name {};

namespace internal {
template <typename Ty>
concept has_alias_name = requires { alias_name<Ty>::value; };
} // namespace internal

template <concepts::pure Ty>
requires(!internal::has_alias_name<Ty>)
[[nodiscard]] consteval inline std::string_view alias_name_of() {
    return name_of<Ty>();
}

template <concepts::pure Ty>
requires internal::has_alias_name<Ty>
[[nodiscard]] consteval inline std::string_view alias_name_of() {
    return alias_name<Ty>::value;
}

} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// module: member_count_of <typename>
///////////////////////////////////////////////////////////////////////////////

namespace atom::utils {
/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

template <typename Ty, typename... Args>
constexpr inline auto member_count_of_impl() {
    if constexpr (std::constructible_from<Ty, Args..., universal>) {
        return member_count_of_impl<Ty, Args..., universal>();
    }
    else {
        return sizeof...(Args);
    }
}

} // namespace internal
/*! @endcond */

/**
 * @brief Get the member count of a type.
 *
 */
template <concepts::reflectible Ty>
consteval inline auto member_count_of() {
    using pure_t = std::remove_cvref_t<Ty>;
    if constexpr (concepts::default_reflectible_aggregate<Ty>) {
        return internal::member_count_of_impl<pure_t>();
    }
    else { // has_field_traits
        return std::tuple_size_v<decltype(pure_t::field_traits())>;
    }
}

template <concepts::reflectible Ty>
constexpr inline size_t member_count_v = member_count_of<Ty>();

} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// module: tuple_view_helper
///////////////////////////////////////////////////////////////////////////////

/*! @cond TURN_OFF_DOXYGEN */
namespace atom::utils::internal {

template <typename Ty>
struct outline {
    [[maybe_unused]] static inline std::remove_cvref_t<Ty> value;
};

template <typename Ty>
constexpr inline const std::remove_cvref_t<Ty>& get_object_outline() noexcept {
    return outline<Ty>::value;
}

template <typename Ty>
struct tuple_view_helper;

// Support for type with 0 member.

template <typename Ty>
requires(member_count_v<Ty> == 0)
struct tuple_view_helper<Ty> {
    constexpr static auto tuple_view() { return std::tuple<>(); }
    constexpr static auto tuple_view(const Ty& obj) { return std::tuple<>(); }
};

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define TUPLE_VIEW_HELPER(N, ...)                                                                  \
    template <typename Ty>                                                                         \
    requires(member_count_v<Ty> == N)                                                              \
    struct tuple_view_helper<Ty> {                                                                 \
        constexpr static auto tuple_view() {                                                       \
            auto& [__VA_ARGS__] = get_object_outline<Ty>();                                        \
            auto tuple          = std::tie(__VA_ARGS__);                                           \
            auto get_ptr        = [](auto&... refs) { return std::make_tuple(&refs...); };         \
            return std::apply(get_ptr, tuple);                                                     \
        }                                                                                          \
        constexpr static auto tuple_view(const Ty& obj) {                                          \
            auto& [__VA_ARGS__] = obj;                                                             \
            return std::tie(__VA_ARGS__);                                                          \
        }                                                                                          \
        constexpr static auto tuple_view(Ty& obj) {                                                \
            auto& [__VA_ARGS__] = obj;                                                             \
            return std::tie(__VA_ARGS__);                                                          \
        }                                                                                          \
    };                                                                                             \
    //

// NOLINTEND(cppcoreguidelines-macro-usage)

// Support for from 1 to 255

TUPLE_VIEW_HELPER(1, _1)
TUPLE_VIEW_HELPER(2, _1, _2)
TUPLE_VIEW_HELPER(3, _1, _2, _3)
TUPLE_VIEW_HELPER(4, _1, _2, _3, _4)
TUPLE_VIEW_HELPER(5, _1, _2, _3, _4, _5)
TUPLE_VIEW_HELPER(6, _1, _2, _3, _4, _5, _6)
TUPLE_VIEW_HELPER(7, _1, _2, _3, _4, _5, _6, _7)
TUPLE_VIEW_HELPER(8, _1, _2, _3, _4, _5, _6, _7, _8)
TUPLE_VIEW_HELPER(9, _1, _2, _3, _4, _5, _6, _7, _8, _9)
TUPLE_VIEW_HELPER(10, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10)
TUPLE_VIEW_HELPER(11, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11)
TUPLE_VIEW_HELPER(12, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)
TUPLE_VIEW_HELPER(13, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13)
TUPLE_VIEW_HELPER(14, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14)
TUPLE_VIEW_HELPER(15, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15)
TUPLE_VIEW_HELPER(16, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16)
TUPLE_VIEW_HELPER(17, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17)
TUPLE_VIEW_HELPER(
    18, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18)
TUPLE_VIEW_HELPER(
    19, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19)
TUPLE_VIEW_HELPER(
    20, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20)
TUPLE_VIEW_HELPER(
    21, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21)
TUPLE_VIEW_HELPER(
    22, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22)
TUPLE_VIEW_HELPER(
    23, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23)
TUPLE_VIEW_HELPER(
    24, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24)
TUPLE_VIEW_HELPER(
    25, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25)
TUPLE_VIEW_HELPER(
    26, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26)
TUPLE_VIEW_HELPER(
    27, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27)
TUPLE_VIEW_HELPER(
    28, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28)
TUPLE_VIEW_HELPER(
    29, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29)
TUPLE_VIEW_HELPER(
    30, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30)
TUPLE_VIEW_HELPER(
    31, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31)
TUPLE_VIEW_HELPER(
    32, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32)
TUPLE_VIEW_HELPER(
    33, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33)
TUPLE_VIEW_HELPER(
    34, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34)
TUPLE_VIEW_HELPER(
    35, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35)
TUPLE_VIEW_HELPER(
    36, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36)
TUPLE_VIEW_HELPER(
    37, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37)
TUPLE_VIEW_HELPER(
    38, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38)
TUPLE_VIEW_HELPER(
    39, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39)
TUPLE_VIEW_HELPER(
    40, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40)
TUPLE_VIEW_HELPER(
    41, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41)
TUPLE_VIEW_HELPER(
    42, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42)
TUPLE_VIEW_HELPER(
    43, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43)
TUPLE_VIEW_HELPER(
    44, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44)
TUPLE_VIEW_HELPER(
    45, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45)
TUPLE_VIEW_HELPER(
    46, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46)
TUPLE_VIEW_HELPER(
    47, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47)
TUPLE_VIEW_HELPER(
    48, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48)
TUPLE_VIEW_HELPER(
    49, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49)
TUPLE_VIEW_HELPER(
    50, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50)
TUPLE_VIEW_HELPER(
    51, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51)
TUPLE_VIEW_HELPER(
    52, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52)
TUPLE_VIEW_HELPER(
    53, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53)
TUPLE_VIEW_HELPER(
    54, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54)
TUPLE_VIEW_HELPER(
    55, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55)
TUPLE_VIEW_HELPER(
    56, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56)
TUPLE_VIEW_HELPER(
    57, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57)
TUPLE_VIEW_HELPER(
    58, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58)
TUPLE_VIEW_HELPER(
    59, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59)
TUPLE_VIEW_HELPER(
    60, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60)
TUPLE_VIEW_HELPER(
    61, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61)
TUPLE_VIEW_HELPER(
    62, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62)
TUPLE_VIEW_HELPER(
    63, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63)
TUPLE_VIEW_HELPER(
    64, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64)
TUPLE_VIEW_HELPER(
    65, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65)
TUPLE_VIEW_HELPER(
    66, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66)
TUPLE_VIEW_HELPER(
    67, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67)
TUPLE_VIEW_HELPER(
    68, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68)
TUPLE_VIEW_HELPER(
    69, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69)
TUPLE_VIEW_HELPER(
    70, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70)
TUPLE_VIEW_HELPER(
    71, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71)
TUPLE_VIEW_HELPER(
    72, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72)
TUPLE_VIEW_HELPER(
    73, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73)
TUPLE_VIEW_HELPER(
    74, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74)
TUPLE_VIEW_HELPER(
    75, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75)
TUPLE_VIEW_HELPER(
    76, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76)
TUPLE_VIEW_HELPER(
    77, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77)
TUPLE_VIEW_HELPER(
    78, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78)
TUPLE_VIEW_HELPER(
    79, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79)
TUPLE_VIEW_HELPER(
    80, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80)
TUPLE_VIEW_HELPER(
    81, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81)
TUPLE_VIEW_HELPER(
    82, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82)
TUPLE_VIEW_HELPER(
    83, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83)
TUPLE_VIEW_HELPER(
    84, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84)
TUPLE_VIEW_HELPER(
    85, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85)
TUPLE_VIEW_HELPER(
    86, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86)
TUPLE_VIEW_HELPER(
    87, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87)
TUPLE_VIEW_HELPER(
    88, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88)
TUPLE_VIEW_HELPER(
    89, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89)
TUPLE_VIEW_HELPER(
    90, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90)
TUPLE_VIEW_HELPER(
    91, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91)
TUPLE_VIEW_HELPER(
    92, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92)
TUPLE_VIEW_HELPER(
    93, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93)
TUPLE_VIEW_HELPER(
    94, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94)
TUPLE_VIEW_HELPER(
    95, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95)
TUPLE_VIEW_HELPER(
    96, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96)
TUPLE_VIEW_HELPER(
    97, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97)
TUPLE_VIEW_HELPER(
    98, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98)
TUPLE_VIEW_HELPER(
    99, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99)
TUPLE_VIEW_HELPER(
    100, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100)
TUPLE_VIEW_HELPER(
    101, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101)
TUPLE_VIEW_HELPER(
    102, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102)
TUPLE_VIEW_HELPER(
    103, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103)
TUPLE_VIEW_HELPER(
    104, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104)
TUPLE_VIEW_HELPER(
    105, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105)
TUPLE_VIEW_HELPER(
    106, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106)
TUPLE_VIEW_HELPER(
    107, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107)
TUPLE_VIEW_HELPER(
    108, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108)
TUPLE_VIEW_HELPER(
    109, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109)
TUPLE_VIEW_HELPER(
    110, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110)
TUPLE_VIEW_HELPER(
    111, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111)
TUPLE_VIEW_HELPER(
    112, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112)
TUPLE_VIEW_HELPER(
    113, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113)
TUPLE_VIEW_HELPER(
    114, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114)
TUPLE_VIEW_HELPER(
    115, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115)
TUPLE_VIEW_HELPER(
    116, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116)
TUPLE_VIEW_HELPER(
    117, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117)
TUPLE_VIEW_HELPER(
    118, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117, _118)
TUPLE_VIEW_HELPER(
    119, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117, _118, _119)
TUPLE_VIEW_HELPER(
    120, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117, _118, _119, _120)
TUPLE_VIEW_HELPER(
    121, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117, _118, _119, _120, _121)
TUPLE_VIEW_HELPER(
    122, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117, _118, _119, _120, _121, _122)
TUPLE_VIEW_HELPER(
    123, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117, _118, _119, _120, _121, _122, _123)
TUPLE_VIEW_HELPER(
    124, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117, _118, _119, _120, _121, _122, _123, _124)
TUPLE_VIEW_HELPER(
    125, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117, _118, _119, _120, _121, _122, _123, _124, _125)
TUPLE_VIEW_HELPER(
    126, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117, _118, _119, _120, _121, _122, _123, _124, _125, _126)
TUPLE_VIEW_HELPER(
    127, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39,
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58,
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77,
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96,
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,
    _113, _114, _115, _116, _117, _118, _119, _120, _121, _122, _123, _124, _125, _126, _127)

template <typename Ty>
constexpr inline auto struct_to_tuple_view() {
    return internal::tuple_view_helper<std::remove_cvref_t<Ty>>::tuple_view();
}

template <typename Ty>
constexpr inline auto object_to_tuple_view(const Ty& obj) {
    return internal::tuple_view_helper<std::remove_cvref_t<Ty>>::tuple_view(obj);
}

template <typename Ty>
constexpr inline auto object_to_tuple_view(Ty& obj) {
    return internal::tuple_view_helper<std::remove_cvref_t<Ty>>::tuple_view(obj);
}

} // namespace atom::utils::internal
/*! @endcond */

///////////////////////////////////////////////////////////////////////////////
// module: member_names_of <typename>
///////////////////////////////////////////////////////////////////////////////

namespace atom::utils {
/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
template <auto Ptr> // name
FORCE_INLINE consteval std::string_view member_name_of() {
#ifdef _MSC_VER
    constexpr std::string_view funcname = __FUNCSIG__;
#else
    constexpr std::string_view funcname = __PRETTY_FUNCTION__;
#endif

#ifdef __clang__
    auto split = funcname.substr(0, funcname.size() - 2);
    return split.substr(split.find_last_of(":.") + 1);
#elif defined(__GNUC__)
    auto split = funcname.substr(0, funcname.rfind(")}"));
    return split.substr(split.find_last_of(":") + 1);
#elif defined(_MSC_VER)
    auto split = funcname.substr(0, funcname.rfind("}>"));
    return split.substr(split.rfind("->") + 2);
#else
    static_assert(false, "Unsupportted compiler");
#endif
}

template <typename Ty>
struct wrapper {
    Ty val;
};

template <typename Ty>
wrapper(Ty) -> wrapper<Ty>;

template <typename Ty>
FORCE_INLINE constexpr auto wrap(const Ty& arg) noexcept {
    return wrapper{ arg };
}

} // namespace internal
/*! @endcond */

/**
 * @brief Get members' name of a type.
 *
 */
template <concepts::reflectible Ty>
constexpr inline std::array<std::string_view, member_count_v<Ty>> member_names_of() {
    constexpr size_t count = member_count_v<Ty>;
    std::array<std::string_view, count> array;
    if constexpr (concepts::default_reflectible_aggregate<Ty>) {
        constexpr auto tuple = internal::struct_to_tuple_view<Ty>();
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            ((array[Is] = internal::member_name_of<internal::wrap(std::get<Is>(tuple))>()), ...);
        }(std::make_index_sequence<count>());
    }
    else { // has_field_traits
        constexpr auto tuple = Ty::field_traits();
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            ((array[Is] = std::get<Is>(tuple).name()), ...);
        }(std::make_index_sequence<count>());
    }
    return array;
}
} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// module: hash_of
///////////////////////////////////////////////////////////////////////////////

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

template <concepts::pure Ty>
constexpr inline size_t hash_of() {
    constexpr auto name = name_of<Ty>();
    return internal::hash(name);
}

inline size_t hash_of(std::string_view str) { return internal::hash(str); }

} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// module: interactive with members
///////////////////////////////////////////////////////////////////////////////

namespace atom::utils {
/**
 * @brief Get member's value.
 *
 */
template <std::size_t Index, concepts::reflectible Ty>
constexpr inline auto& get(Ty&& obj) {
    static_assert(Index < member_count_v<Ty>);
    using pure_t = std::remove_cvref_t<Ty>;

    if constexpr (concepts::default_reflectible_aggregate<Ty>) {
        auto tuple = internal::object_to_tuple_view<pure_t>(std::forward<Ty>(obj));
        return std::get<Index>(tuple);
    }
    else { // has_field_traits
        auto traits = pure_t::field_traits();
        return std::get<Index>(traits).get(obj);
    }
}

/**
 * @brief Get the existance of a member.
 *
 */
template <tstring_v Name, concepts::reflectible Ty>
[[nodiscard]] consteval inline auto existance_of() {
    constexpr auto count = member_count_v<Ty>;
    constexpr auto names = member_names_of<Ty>();
    for (auto i = 0; i < count; ++i) {
        if (names[i] == Name.val) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Get the existance of a member.
 *
 */
template <concepts::reflectible Ty>
[[nodiscard]] constexpr inline auto existance_of(std::string_view name) -> size_t {
    constexpr auto count = member_count_v<Ty>();
    auto names           = member_names_of<Ty>();
    for (auto i = 0; i < count; ++i) {
        if (names[i] == name) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Get the index of a member.
 *
 */
template <tstring_v Name, concepts::reflectible Ty>
[[nodiscard]] consteval inline auto index_of() -> size_t {
    constexpr auto count = member_count_v<Ty>;
    constexpr auto names = member_names_of<Ty>();
    for (auto i = 0; i < count; ++i) {
        if (names[i] == Name.val) {
            return i;
        }
    }
    return static_cast<std::size_t>(-1);
}

/**
 * @brief Get the index of a member.
 *
 */
template <concepts::reflectible Ty>
[[nodiscard]] constexpr inline auto index_of(std::string_view name) -> size_t {
    constexpr auto count = member_count_v<Ty>;
    auto names           = member_names_of<Ty>();
    for (auto i = 0; i < count; ++i) {
        if (names[i] == name) {
            return i;
        }
    }
    return static_cast<std::size_t>(-1);
}

template <size_t Index, concepts::reflectible Ty>
[[nodiscard]] consteval inline bool valid_index() noexcept {
    return Index < member_count_v<Ty>;
}

template <concepts::reflectible Ty>
[[nodiscard]] constexpr inline bool valid_index(const size_t index) noexcept {
    return index < member_count_v<Ty>;
}

/**
 * @brief Get the name of a member.
 *
 */
template <size_t Index, concepts::reflectible Ty>
[[nodiscard]] consteval inline auto name_of() {
    constexpr auto count = member_count_v<Ty>;
    static_assert(Index < count, "Index out of range");
    constexpr auto names = member_names_of<Ty>();
    return names[Index];
}

/**
 * @brief Get the name of a member.
 *
 */
template <concepts::reflectible Ty>
[[nodiscard]] constexpr inline auto name_of(size_t index) {
    constexpr auto names = member_names_of<Ty>();
    return names[index];
}

template <tstring_v Name, typename Ty>
constexpr inline auto& get(Ty& obj) {
    constexpr auto index = index_of<Name, Ty>();
    static_assert(index < member_count_v<Ty>());
    return get<index>(obj);
}

} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// module: description_of <typename>
///////////////////////////////////////////////////////////////////////////////

namespace atom::utils {

using description_bits_base = std::uint64_t;

namespace bits {
// clang-format off
enum description_bits : description_bits_base {
    is_integral                        = 0b0000000000000000000000000000000000000000000000000000000000000001,
    is_floating_point                  = 0b0000000000000000000000000000000000000000000000000000000000000010,
    is_enum                            = 0b0000000000000000000000000000000000000000000000000000000000000100,
    is_union                           = 0b0000000000000000000000000000000000000000000000000000000000001000,
    is_class                           = 0b0000000000000000000000000000000000000000000000000000000000010000,
    is_object                          = 0b0000000000000000000000000000000000000000000000000000000000100000,
    is_trivial                         = 0b0000000000000000000000000000000000000000000000000000000001000000,
    is_standard_layout                 = 0b0000000000000000000000000000000000000000000000000000000010000000,
    is_empty                           = 0b0000000000000000000000000000000000000000000000000000000100000000,
    is_polymorphic                     = 0b0000000000000000000000000000000000000000000000000000001000000000,
    is_abstract                        = 0b0000000000000000000000000000000000000000000000000000010000000000,
    is_final                           = 0b0000000000000000000000000000000000000000000000000000100000000000,
    is_aggregate                       = 0b0000000000000000000000000000000000000000000000000001000000000000,
    is_function                        = 0b0000000000000000000000000000000000000000000000000010000000000000,
    is_default_constructible           = 0b0000000000000000000000000000000000000000000000000100000000000000,
    is_trivially_default_constructible = 0b0000000000000000000000000000000000000000000000001000000000000000,
    is_nothrow_default_constructible   = 0b0000000000000000000000000000000000000000000000010000000000000000,
    is_copy_constructible              = 0b0000000000000000000000000000000000000000000000100000000000000000,
    is_nothrow_copy_constructible      = 0b0000000000000000000000000000000000000000000001000000000000000000,
    is_trivially_copy_constructible    = 0b0000000000000000000000000000000000000000000010000000000000000000,
    is_move_constructible              = 0b0000000000000000000000000000000000000000000100000000000000000000,
    is_trivially_move_constructible    = 0b0000000000000000000000000000000000000000001000000000000000000000,
    is_nothrow_move_construcitble      = 0b0000000000000000000000000000000000000000010000000000000000000000,
    is_copy_assignable                 = 0b0000000000000000000000000000000000000000100000000000000000000000,
    is_trivially_copy_assignable       = 0b0000000000000000000000000000000000000001000000000000000000000000,
    is_nothrow_copy_assignable         = 0b0000000000000000000000000000000000000010000000000000000000000000,
    is_move_assignable                 = 0b0000000000000000000000000000000000000100000000000000000000000000,
    is_trivially_move_assignable       = 0b0000000000000000000000000000000000001000000000000000000000000000,
    is_nothrow_move_assignable         = 0b0000000000000000000000000000000000010000000000000000000000000000,
    is_destructible                    = 0b0000000000000000000000000000000000100000000000000000000000000000,
    is_trivially_destructible          = 0b0000000000000000000000000000000001000000000000000000000000000000,
    is_nothrow_destructible            = 0b0000000000000000000000000000000010000000000000000000000000000000,
    // reserve
    _reserve                           = 0xFFFFFFFFFFFFFFFF
};
// clang-format on
} // namespace bits
using description_bits = bits::description_bits;

template <concepts::pure Ty>
consteval inline auto description_of() noexcept -> description_bits {
    using namespace bits;
    description_bits_base mask = 0;
    mask |= std::is_integral_v<Ty> ? is_integral : 0;
    mask |= std::is_floating_point_v<Ty> ? is_floating_point : 0;
    mask |= std::is_enum_v<Ty> ? is_enum : 0;
    mask |= std::is_union_v<Ty> ? is_union : 0;
    mask |= std::is_class_v<Ty> ? is_class : 0;
    mask |= std::is_object_v<Ty> ? is_object : 0;
    mask |= std::is_trivial_v<Ty> ? is_trivial : 0;
    mask |= std::is_standard_layout_v<Ty> ? is_standard_layout : 0;
    mask |= std::is_empty_v<Ty> ? is_empty : 0;
    mask |= std::is_polymorphic_v<Ty> ? is_polymorphic : 0;
    mask |= std::is_abstract_v<Ty> ? is_abstract : 0;
    mask |= std::is_final_v<Ty> ? is_final : 0;
    mask |= std::is_aggregate_v<Ty> ? is_aggregate : 0;
    mask |= std::is_function_v<Ty> ? is_function : 0;
    mask |= std::is_default_constructible_v<Ty> ? is_default_constructible : 0;
    mask |= std::is_trivially_default_constructible_v<Ty> ? is_trivially_default_constructible : 0;
    mask |= std::is_nothrow_default_constructible_v<Ty> ? is_nothrow_default_constructible : 0;
    mask |= std::is_copy_constructible_v<Ty> ? is_copy_constructible : 0;
    mask |= std::is_trivially_copy_constructible_v<Ty> ? is_trivially_default_constructible : 0;
    mask |= std::is_nothrow_copy_constructible_v<Ty> ? is_nothrow_copy_constructible : 0;
    mask |= std::is_move_constructible_v<Ty> ? is_move_constructible : 0;
    mask |= std::is_trivially_move_constructible_v<Ty> ? is_trivially_move_constructible : 0;
    mask |= std::is_nothrow_move_constructible_v<Ty> ? is_nothrow_move_construcitble : 0;
    mask |= std::is_copy_assignable_v<Ty> ? is_copy_assignable : 0;
    mask |= std::is_trivially_copy_assignable_v<Ty> ? is_trivially_copy_assignable : 0;
    mask |= std::is_nothrow_copy_assignable_v<Ty> ? is_nothrow_copy_assignable : 0;
    mask |= std::is_move_assignable_v<Ty> ? is_move_assignable : 0;
    mask |= std::is_trivially_move_assignable_v<Ty> ? is_trivially_move_assignable : 0;
    mask |= std::is_nothrow_move_assignable_v<Ty> ? is_nothrow_move_assignable : 0;
    mask |= std::is_destructible_v<Ty> ? is_destructible : 0;
    mask |= std::is_trivially_destructible_v<Ty> ? is_trivially_destructible : 0;
    mask |= std::is_nothrow_destructible_v<Ty> ? is_nothrow_destructible : 0;
    return static_cast<description_bits>(mask);
}

template <concepts::pure Ty>
constexpr inline bool authenticity_of(const description_bits bits) noexcept {
    constexpr auto description = static_cast<description_bits_base>(description_of<Ty>());
    auto mask                  = static_cast<description_bits_base>(bits);
    auto result                = description & mask;
    return result == mask;
}

constexpr inline bool authenticity_of(
    const description_bits description, const description_bits bits) noexcept {
    auto mask   = static_cast<description_bits_base>(bits);
    auto result = static_cast<description_bits_base>(description) & mask;
    return result == mask;
}

} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// reflected
///////////////////////////////////////////////////////////////////////////////

namespace atom::utils {

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
constexpr static inline std::string_view empty_string = "";
constexpr static inline std::tuple<> empty_tuple{};
} // namespace internal
/*! @endcond */

/**
 * @brief Recording basic information about a type.
 *
 */
struct basic_reflected {
public:
    constexpr basic_reflected(const basic_reflected&) noexcept            = default;
    constexpr basic_reflected& operator=(const basic_reflected&) noexcept = default;

    constexpr basic_reflected(basic_reflected&& obj) noexcept
        : name_(obj.name_), hash_(obj.hash_), description_(obj.description_) {}
    constexpr basic_reflected& operator=(basic_reflected&& obj) noexcept {
        name_        = obj.name_;
        hash_        = obj.hash_;
        description_ = obj.description_;
        destroy_     = obj.destroy_;
        return *this;
    }

    constexpr ~basic_reflected() noexcept = default;

    /**
     * @brief Get the name of this reflected type.
     *
     * @return std::string_view The name.
     */
    [[nodiscard]] constexpr auto name() const noexcept -> std::string_view { return name_; }

    [[nodiscard]] constexpr auto alias_name() const noexcept -> std::string_view {
        return alias_name_;
    }

    /**
     * @brief Get the hash value of this reflected type.
     *
     * @return std::size_t
     */
    [[nodiscard]] constexpr auto hash() const noexcept -> std::size_t { return hash_; }

    [[nodiscard]] constexpr auto description() const noexcept -> description_bits {
        return description_;
    }

    constexpr void destroy(void* ptr) const {
        if (destroy_) {
            destroy_(ptr);
        }
    }

protected:
    constexpr explicit basic_reflected(
        std::string_view name, const std::size_t hash, const description_bits description,
        std::string_view alias_name, void (*destroy)(void*) = nullptr) noexcept
        : name_(name), hash_(hash), description_(description), destroy_(destroy) {}

private:
    std::string_view name_;
    std::string_view alias_name_;
    std::size_t hash_;
    description_bits description_;
    void (*destroy_)(void*) = nullptr;
};

/**
 * @brief A type stores reflected information.
 *
 * @tparam Ty The reflected type.
 */
template <concepts::pure Ty>
struct reflected final : public basic_reflected {
    constexpr reflected() noexcept
        : basic_reflected(
              name_of<Ty>(), hash_of<Ty>(), description_of<Ty>(), alias_name_of<Ty>(),
              &internal::wrapped_destroy<Ty>) {}

    reflected(const reflected&) noexcept            = default;
    reflected(reflected&&) noexcept                 = default;
    reflected& operator=(const reflected&) noexcept = default;
    reflected& operator=(reflected&&) noexcept      = default;
    constexpr ~reflected() noexcept                 = default;

    /**
     * @brief Fields exploded to outside in the type.
     *
     * @return constexpr const auto& A tuple contains function traits.
     */
    constexpr const auto& fields() const noexcept {
        if constexpr (concepts::aggregate<Ty>) {}
        else if constexpr (concepts::has_field_traits<Ty>) {
            return Ty::field_traits();
        }
        else {
            return internal::empty_tuple;
        }
    }

    /**
     * @brief Functions exploded to outside in the type.
     *
     * @return constexpr const auto& A tuple contains function traits.
     */
    constexpr const auto& functions() const noexcept {
        if constexpr (concepts::has_function_traits<Ty>) {
            return Ty::function_traits();
        }
        else {
            return internal::empty_tuple;
        }
    }
};

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
template <::atom::utils::tstring_v Name, std::size_t Index, typename Tuple>
constexpr void find_traits(const Tuple& tuple, std::size_t& result) {
    if (std::get<Index>(tuple).name() == Name.val) {
        result = Index;
    }
}
} // namespace internal
/*! @endcond */

template <::atom::utils::tstring_v Name, typename Tuple>
consteval std::size_t index_of(const Tuple& tuple) {
    auto index_sequence = std::make_index_sequence<std::tuple_size_v<Tuple>>();
    std::size_t result  = std::tuple_size_v<Tuple>;
    []<std::size_t... Is>(const Tuple& tuple, std::size_t& result, std::index_sequence<Is...>) {
        (internal::find_traits<Name, Is>(tuple, result), ...);
    }(tuple, result, index_sequence);
    return result;
}

} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// registry & register
///////////////////////////////////////////////////////////////////////////////

#include <shared_mutex>
#include <unordered_map>

namespace atom::utils {
/**
 * @brief Type information registry.
 *
 * Gather reflected information.
 */
template <typename Placeholder>
class registry {
    using self_type = registry;
    using pointer   = std::shared_ptr<basic_reflected>;

public:
    registry() = delete;

    /**
     * @brief Get unique identity.
     *
     * @param hash The hash value of a type.
     * @return default_id_t Unique identity.
     */
    static default_id_t identity(const size_t hash) {
        static std::unordered_map<size_t, default_id_t> map;
        if (!map.contains(hash)) {
            map[hash] = next_id();
        }

        return map.at(hash);
    }

    /**
     * @brief Register a type.
     *
     * @tparam Ty The type want to register.
     */
    template <typename Ty>
    static void enroll() {
        using pure_t     = typename std::remove_cvref_t<Ty>;
        const auto hash  = hash_of<pure_t>();
        const auto ident = identity(hash);

        auto& registered = self_type::registered();
        auto& mutex      = self_type::mutex();
        std::shared_lock<std::shared_mutex> shared_lock{ mutex };
        if (!registered.contains(ident)) [[likely]] {
            shared_lock.unlock();
            std::unique_lock<std::shared_mutex> unique_lock{ mutex };
            registered.emplace(ident, std::make_shared<reflected<pure_t>>());
            unique_lock.unlock();
            shared_lock.lock();
        }
    }

    /**
     * @brief Fint out the reflected information.
     *
     * @param ident Unique identity of a type.
     * @return auto Shared pointer.
     */
    static auto find(const default_id_t ident) -> std::shared_ptr<::atom::utils::basic_reflected> {
        auto& registered = self_type::registered();
        auto& mutex      = self_type::mutex();
        std::shared_lock<std::shared_mutex> shared_lock{ mutex };
        if (auto iter = registered.find(ident); iter != registered.cend()) [[likely]] {
            return iter->second;
        }
        else [[unlikely]] {
            // Remember some basic classes were not registered automatically, like uint32_t (usually
            // aka unsigned int). You could see these in reflection/macros.hpp
            //
            // Notice: Please make sure you have already registered the type you want to reflect
            // when calling this.
            throw std::runtime_error("Unregistered type!");
        }
    }

    template <typename Ty>
    static auto find() -> std::shared_ptr<basic_reflected> {
        using pure_t     = std::remove_cvref_t<Ty>;
        const auto hash  = hash_of<pure_t>();
        const auto ident = identity(hash);
        return find(ident);
    }

    static auto all() {
        auto& registered = self_type::registered();
        return registered | std::views::values;
    }

protected:
    static inline auto registered()
        -> std::unordered_map<default_id_t, std::shared_ptr<basic_reflected>>& {
        static std::unordered_map<default_id_t, pointer> registered;
        return registered;
    }

    static inline auto mutex() -> std::shared_mutex& {
        static std::shared_mutex mutex;
        return mutex;
    }

    static inline auto next_id() -> default_id_t {
        static std::atomic<default_id_t> current_id;
        auto next_id = current_id.load(std::memory_order_acquire);
        current_id.fetch_add(1, std::memory_order_release);
        return next_id;
    }
};
} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// module: macros for user
///////////////////////////////////////////////////////////////////////////////

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define EXPAND(...) __VA_ARGS__

#define NUM_ARGS_HELPER_(                                                                          \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20,     \
    _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, \
    _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, \
    _59, _60, _61, _62, _63, _64, _65, _66, _67, _68, _69, _70, _71, _72, _73, _74, _75, _76, _77, \
    _78, _79, _80, _81, _82, _83, _84, _85, _86, _87, _88, _89, _90, _91, _92, _93, _94, _95, _96, \
    _97, _98, _99, _100, _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, _111, _112,   \
    _113, _114, _115, _116, _117, _118, _119, _120, _121, _122, _123, _124, _125, N, ...)          \
    N

#define NUM_ARGS_HELPER(...)                                                                       \
    EXPAND(NUM_ARGS_HELPER_(                                                                       \
        __VA_ARGS__, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111,    \
        110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, \
        90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69,    \
        68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47,    \
        46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25,    \
        24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))

#define NUM_ARGS(...) NUM_ARGS_HELPER(__VA_ARGS__)

#define CONCAT_(l, r) l##r
#define CONCAT(l, r) CONCAT_(l, r)

#define FIELD_TRAITS(_, x)                                                                         \
    ::atom::utils::field_traits<decltype(&_::x)> { #x, &_::x }                                     \
    //

#define FUNCTION_TRAITS(_, x)                                                                      \
    ::atom::utils::function_traits<decltype(&_::x)> { #x, &_::x }                                  \
    //

#define FOR_EACH_0(MACRO, _)
#define FOR_EACH_1(MACRO, _, _1) MACRO(_, _1)
#define FOR_EACH_2(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_1(MACRO, _, __VA_ARGS__))
#define FOR_EACH_3(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_2(MACRO, _, __VA_ARGS__))
#define FOR_EACH_4(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_3(MACRO, _, __VA_ARGS__))
#define FOR_EACH_5(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_4(MACRO, _, __VA_ARGS__))
#define FOR_EACH_6(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_5(MACRO, _, __VA_ARGS__))
#define FOR_EACH_7(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_6(MACRO, _, __VA_ARGS__))
#define FOR_EACH_8(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_7(MACRO, _, __VA_ARGS__))
#define FOR_EACH_9(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_8(MACRO, _, __VA_ARGS__))
#define FOR_EACH_10(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_9(MACRO, _, __VA_ARGS__))
#define FOR_EACH_11(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_10(MACRO, _, __VA_ARGS__))
#define FOR_EACH_12(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_11(MACRO, _, __VA_ARGS__))
#define FOR_EACH_13(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_12(MACRO, _, __VA_ARGS__))
#define FOR_EACH_14(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_13(MACRO, _, __VA_ARGS__))
#define FOR_EACH_15(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_14(MACRO, _, __VA_ARGS__))
#define FOR_EACH_16(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_15(MACRO, _, __VA_ARGS__))
#define FOR_EACH_17(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_16(MACRO, _, __VA_ARGS__))
#define FOR_EACH_18(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_17(MACRO, _, __VA_ARGS__))
#define FOR_EACH_19(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_18(MACRO, _, __VA_ARGS__))
#define FOR_EACH_20(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_19(MACRO, _, __VA_ARGS__))
#define FOR_EACH_21(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_20(MACRO, _, __VA_ARGS__))
#define FOR_EACH_22(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_21(MACRO, _, __VA_ARGS__))
#define FOR_EACH_23(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_22(MACRO, _, __VA_ARGS__))
#define FOR_EACH_24(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_23(MACRO, _, __VA_ARGS__))
#define FOR_EACH_25(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_24(MACRO, _, __VA_ARGS__))
#define FOR_EACH_26(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_25(MACRO, _, __VA_ARGS__))
#define FOR_EACH_27(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_26(MACRO, _, __VA_ARGS__))
#define FOR_EACH_28(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_27(MACRO, _, __VA_ARGS__))
#define FOR_EACH_29(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_28(MACRO, _, __VA_ARGS__))
#define FOR_EACH_30(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_29(MACRO, _, __VA_ARGS__))
#define FOR_EACH_31(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_30(MACRO, _, __VA_ARGS__))
#define FOR_EACH_32(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_31(MACRO, _, __VA_ARGS__))
#define FOR_EACH_33(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_32(MACRO, _, __VA_ARGS__))
#define FOR_EACH_34(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_33(MACRO, _, __VA_ARGS__))
#define FOR_EACH_35(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_34(MACRO, _, __VA_ARGS__))
#define FOR_EACH_36(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_35(MACRO, _, __VA_ARGS__))
#define FOR_EACH_37(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_36(MACRO, _, __VA_ARGS__))
#define FOR_EACH_38(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_37(MACRO, _, __VA_ARGS__))
#define FOR_EACH_39(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_38(MACRO, _, __VA_ARGS__))
#define FOR_EACH_40(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_39(MACRO, _, __VA_ARGS__))
#define FOR_EACH_41(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_40(MACRO, _, __VA_ARGS__))
#define FOR_EACH_42(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_41(MACRO, _, __VA_ARGS__))
#define FOR_EACH_43(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_42(MACRO, _, __VA_ARGS__))
#define FOR_EACH_44(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_43(MACRO, _, __VA_ARGS__))
#define FOR_EACH_45(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_44(MACRO, _, __VA_ARGS__))
#define FOR_EACH_46(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_45(MACRO, _, __VA_ARGS__))
#define FOR_EACH_47(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_46(MACRO, _, __VA_ARGS__))
#define FOR_EACH_48(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_47(MACRO, _, __VA_ARGS__))
#define FOR_EACH_49(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_48(MACRO, _, __VA_ARGS__))
#define FOR_EACH_50(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_49(MACRO, _, __VA_ARGS__))
#define FOR_EACH_51(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_50(MACRO, _, __VA_ARGS__))
#define FOR_EACH_52(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_51(MACRO, _, __VA_ARGS__))
#define FOR_EACH_53(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_52(MACRO, _, __VA_ARGS__))
#define FOR_EACH_54(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_53(MACRO, _, __VA_ARGS__))
#define FOR_EACH_55(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_54(MACRO, _, __VA_ARGS__))
#define FOR_EACH_56(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_55(MACRO, _, __VA_ARGS__))
#define FOR_EACH_57(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_56(MACRO, _, __VA_ARGS__))
#define FOR_EACH_58(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_57(MACRO, _, __VA_ARGS__))
#define FOR_EACH_59(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_58(MACRO, _, __VA_ARGS__))
#define FOR_EACH_60(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_59(MACRO, _, __VA_ARGS__))
#define FOR_EACH_61(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_60(MACRO, _, __VA_ARGS__))
#define FOR_EACH_62(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_61(MACRO, _, __VA_ARGS__))
#define FOR_EACH_63(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_62(MACRO, _, __VA_ARGS__))
#define FOR_EACH_64(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_63(MACRO, _, __VA_ARGS__))
#define FOR_EACH_65(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_64(MACRO, _, __VA_ARGS__))
#define FOR_EACH_66(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_65(MACRO, _, __VA_ARGS__))
#define FOR_EACH_67(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_66(MACRO, _, __VA_ARGS__))
#define FOR_EACH_68(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_67(MACRO, _, __VA_ARGS__))
#define FOR_EACH_69(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_68(MACRO, _, __VA_ARGS__))
#define FOR_EACH_70(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_69(MACRO, _, __VA_ARGS__))
#define FOR_EACH_71(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_70(MACRO, _, __VA_ARGS__))
#define FOR_EACH_72(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_71(MACRO, _, __VA_ARGS__))
#define FOR_EACH_73(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_72(MACRO, _, __VA_ARGS__))
#define FOR_EACH_74(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_73(MACRO, _, __VA_ARGS__))
#define FOR_EACH_75(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_74(MACRO, _, __VA_ARGS__))
#define FOR_EACH_76(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_75(MACRO, _, __VA_ARGS__))
#define FOR_EACH_77(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_76(MACRO, _, __VA_ARGS__))
#define FOR_EACH_78(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_77(MACRO, _, __VA_ARGS__))
#define FOR_EACH_79(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_78(MACRO, _, __VA_ARGS__))
#define FOR_EACH_80(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_79(MACRO, _, __VA_ARGS__))
#define FOR_EACH_81(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_80(MACRO, _, __VA_ARGS__))
#define FOR_EACH_82(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_81(MACRO, _, __VA_ARGS__))
#define FOR_EACH_83(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_82(MACRO, _, __VA_ARGS__))
#define FOR_EACH_84(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_83(MACRO, _, __VA_ARGS__))
#define FOR_EACH_85(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_84(MACRO, _, __VA_ARGS__))
#define FOR_EACH_86(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_85(MACRO, _, __VA_ARGS__))
#define FOR_EACH_87(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_86(MACRO, _, __VA_ARGS__))
#define FOR_EACH_88(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_87(MACRO, _, __VA_ARGS__))
#define FOR_EACH_89(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_88(MACRO, _, __VA_ARGS__))
#define FOR_EACH_90(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_89(MACRO, _, __VA_ARGS__))
#define FOR_EACH_91(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_90(MACRO, _, __VA_ARGS__))
#define FOR_EACH_92(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_91(MACRO, _, __VA_ARGS__))
#define FOR_EACH_93(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_92(MACRO, _, __VA_ARGS__))
#define FOR_EACH_94(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_93(MACRO, _, __VA_ARGS__))
#define FOR_EACH_95(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_94(MACRO, _, __VA_ARGS__))
#define FOR_EACH_96(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_95(MACRO, _, __VA_ARGS__))
#define FOR_EACH_97(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_96(MACRO, _, __VA_ARGS__))
#define FOR_EACH_98(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_97(MACRO, _, __VA_ARGS__))
#define FOR_EACH_99(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_98(MACRO, _, __VA_ARGS__))
#define FOR_EACH_100(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_99(MACRO, _, __VA_ARGS__))
#define FOR_EACH_101(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_100(MACRO, _, __VA_ARGS__))
#define FOR_EACH_102(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_101(MACRO, _, __VA_ARGS__))
#define FOR_EACH_103(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_102(MACRO, _, __VA_ARGS__))
#define FOR_EACH_104(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_103(MACRO, _, __VA_ARGS__))
#define FOR_EACH_105(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_104(MACRO, _, __VA_ARGS__))
#define FOR_EACH_106(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_105(MACRO, _, __VA_ARGS__))
#define FOR_EACH_107(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_106(MACRO, _, __VA_ARGS__))
#define FOR_EACH_108(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_107(MACRO, _, __VA_ARGS__))
#define FOR_EACH_109(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_108(MACRO, _, __VA_ARGS__))
#define FOR_EACH_110(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_109(MACRO, _, __VA_ARGS__))
#define FOR_EACH_111(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_110(MACRO, _, __VA_ARGS__))
#define FOR_EACH_112(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_111(MACRO, _, __VA_ARGS__))
#define FOR_EACH_113(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_112(MACRO, _, __VA_ARGS__))
#define FOR_EACH_114(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_113(MACRO, _, __VA_ARGS__))
#define FOR_EACH_115(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_114(MACRO, _, __VA_ARGS__))
#define FOR_EACH_116(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_115(MACRO, _, __VA_ARGS__))
#define FOR_EACH_117(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_116(MACRO, _, __VA_ARGS__))
#define FOR_EACH_118(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_117(MACRO, _, __VA_ARGS__))
#define FOR_EACH_119(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_118(MACRO, _, __VA_ARGS__))
#define FOR_EACH_120(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_119(MACRO, _, __VA_ARGS__))
#define FOR_EACH_121(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_120(MACRO, _, __VA_ARGS__))
#define FOR_EACH_122(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_121(MACRO, _, __VA_ARGS__))
#define FOR_EACH_123(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_122(MACRO, _, __VA_ARGS__))
#define FOR_EACH_124(MACRO, _, _1, ...) MACRO(_, _1), EXPAND(FOR_EACH_123(MACRO, _, __VA_ARGS__))

#define FOR_EACH_N(MACRO, _, ...) CONCAT(FOR_EACH_, NUM_ARGS(__VA_ARGS__))(MACRO, _, ##__VA_ARGS__)
#define FOR_EACH(MACRO, _, ...) FOR_EACH_N(MACRO, _, ##__VA_ARGS__)

#define REGISTER(type_name, register_name)                                                         \
    namespace _internal::type_registers {                                                          \
    static inline const ::atom::utils::type_register<type_name>(register_name);                    \
    }                                                                                              \
    //

// This macro expands specified structure, so you should make it in global namespace.
#define REFL_NAME(_, name)                                                                         \
    template <>                                                                                    \
    struct ::atom::utils::nickname<_> {                                                            \
        constexpr static inline std::string_view value = #name;                                    \
    }

// This macro expands function, so you should make it public.

// If you meet something wrong when using msvc, pleace try hover your cursor on the macro you wrote,
// and click expand inline
// 如果你在使用MSVC的时候遇到了一些问题，请尝试把你的光标悬浮在你写的宏上，然后点击展开内联
#define REFL_MEMBERS(_, ...)                                                                       \
    constexpr static inline auto field_traits() noexcept {                                         \
        return std::make_tuple(FOR_EACH(FIELD_TRAITS, _, ##__VA_ARGS__));                          \
    } //

// This macro expands function, so you should make it public.

// If you meet something wrong when using msvc, pleace try hover your cursor on the macro you wrote,
// and click expand inline
// 如果你在使用MSVC的时候遇到了一些问题，请尝试把你的光标悬浮在你写的宏上，然后点击展开内联
#define REFL_FUNCS(_, ...)                                                                         \
    constexpr static inline auto function_traits() noexcept {                                      \
        return std::make_tuple(FOR_EACH(FUNCTION_TRAITS, _, ##__VA_ARGS__));                       \
    } //

// NOLINTEND(cppcoreguidelines-macro-usage)

#if __has_include(<nlohmann/json.hpp>)
    #include <nlohmann/json.hpp>

template <::atom::utils::concepts::reflectible Ty>
inline void to_json(nlohmann::json& json, const Ty& obj) {
    constexpr auto names = atom::utils::member_names_of<Ty>();
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((json[names[Is]] = ::atom::utils::get<Is>(obj)), ...);
    }(std::make_index_sequence<atom::utils::member_count_v<Ty>>());
}

template <::atom::utils::concepts::reflectible Ty>
inline void from_json(const nlohmann::json& json, Ty& obj) {
    constexpr auto names = atom::utils::member_names_of<Ty>();
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((json.at(names[Is]).get_to(::atom::utils::get<Is>(obj))), ...);
    }(std::make_index_sequence<atom::utils::member_count_v<Ty>>());
}

#endif

// some errors would be caused when compile simdjson.h with gcc
#if !defined(__GNUC__) && __has_include(<simdjson.h>)
    #include <string>
    #include <string_view>
    #include <simdjson.h>

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
template <size_t Index, typename Ty, typename Val>
inline auto tag_invoke_impl(Ty&& obj, Val& val) {
    using type = std::remove_cvref_t<Val>;
    if constexpr (std::is_same_v<type, uint64_t>) {
        return obj.get_uint64(val);
    }
    else if constexpr (std::is_same_v<type, int64_t>) {
        return obj.get_int64(val);
    }
    else if constexpr (std::is_same_v<type, double>) {
        return obj.get_double(val);
    }
    else if constexpr (std::is_same_v<type, bool>) {
        return obj.get_bool(val);
    }
    else if constexpr (
        std::is_same_v<type, std::string_view> || std::is_same_v<type, std::string>) {
        return obj.get_string(val);
    }
    else if constexpr (std::is_same_v<type, simdjson::ondemand::object>) {
        return obj.get_object(val);
    }
    else if constexpr (std::is_same_v<type, simdjson::ondemand::array>) {
        return obj.get_array(val);
    }
    else {
        return obj.get(val);
    }
}
} // namespace internal
/*! @endcond */

namespace simdjson {
/**
 * @brief Deserialization support for simdjson
 *
 */
template <typename simdjson_value, ::atom::utils::concepts::reflectible Ty>
inline auto tag_invoke(
    simdjson::deserialize_tag, simdjson_value& val,
    Ty& object) noexcept // it would return error code
{
    ondemand::object obj;
    auto error = val.get_object().get(obj);
    if (error) [[unlikely]] {
        return error;
    }

    constexpr auto names = ::atom::utils::member_names_of<Ty>();
    [&]<size_t... Is>(::std::index_sequence<Is...>) {
        (::internal::tag_invoke_impl<Is>(obj[names[Is]], ::atom::utils::get<Is>(object)), ...);
    }(::std::make_index_sequence<atom::utils::member_count_v<Ty>>());

    return simdjson::SUCCESS;
}
} // namespace simdjson

#endif

#if __has_include(<lua.hpp>) && __has_include(<sol/sol.hpp>)
    #include <type_traits>
    #include <sol/sol.hpp>
    #include "reflection/name.hpp"
    #include "reflection/reflected.hpp"

namespace internal {
template <::atom::utils::concepts::reflectible Ty>
inline nlohmann::json to_json(const Ty& obj) {
    nlohmann::json json;
    ::to_json(json, obj);
    return json;
}

template <::atom::utils::concepts::reflectible Ty>
inline void from_json(Ty& obj, const nlohmann::json& json) {
    ::from_json(json, obj);
}
} // namespace internal

template <::atom::utils::concepts::pure Ty>
inline sol::usertype<Ty> bind_to_lua(sol::state& lua) noexcept {
    constexpr auto name = ::atom::utils::name_of<Ty>();
    auto usertype       = lua.new_usertype<Ty>();

    if constexpr (std::is_default_constructible_v<Ty>) {
        usertype["new"] = sol::constructors<Ty()>();
    }

    constexpr auto reflected = ::atom::utils::reflected<Ty>();
    if constexpr (::atom::utils::concepts::has_field_traits<Ty>) {
        const auto& fields = reflected.fields();
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            ((usertype[std::get<Is>(fields).name()] = std::get<Is>(fields).pointer), ...);
        }(std::make_index_sequence<std::tuple_size_v<decltype(fields)>>());

    #if __has_include(<nlohmann/json.hpp>)
        usertype["to_json"]   = &internal::to_json<Ty>;
        usertype["from_json"] = &internal::from_json<Ty>;
    #endif
    }

    if constexpr (::atom::utils::concepts::has_function_traits<Ty>) {
        const auto& funcs = reflected.functions();
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            ((usertype[std::get<Is>(funcs).name()] = std::get<Is>(funcs).pointer()), ...);
        }(std::make_index_sequence<std::tuple_size_v<decltype(funcs)>>());
    }
}

#endif
