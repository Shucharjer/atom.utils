/**
 * @file reflection.hpp
 * @brief A one header file reflection framework.
 * @date 2025-02-21
 *
 */
#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include <tuple>
#include <utility>

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
    constexpr tstring_v(const char (&arr)[N]) noexcept {
        // memcpy is not a compile-time function
        // std::memcpy(val, arr, N);
        std::copy(arr, arr + N, val);
    }

    template <std::size_t Num>
    constexpr auto operator<=>(const tstring_v<Num>& obj) const noexcept {
        return std::strcmp(val, obj.val);
    }

    template <std::size_t Num>
    constexpr auto operator==(const tstring_v<Num>& obj) const noexcept -> bool {
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

template <typename Ty>
constexpr void wrapped_destroy(void* ptr) noexcept(noexcept(destroy(static_cast<Ty*>(ptr)))) {
    std::destroy_at(static_cast<Ty*>(ptr));
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
    explicit constexpr basic_field_traits(const char* name) noexcept : name_(name) {}
    explicit constexpr basic_field_traits(std::string_view name) noexcept : name_(name) {}

    constexpr basic_field_traits(const basic_field_traits&) noexcept            = default;
    constexpr basic_field_traits& operator=(const basic_field_traits&) noexcept = default;

    constexpr basic_field_traits(basic_field_traits&& obj) noexcept : name_(obj.name_) {}
    constexpr basic_field_traits& operator=(basic_field_traits&& obj) noexcept {
        if (this != &obj) {
            name_ = obj.name_;
        }
        return *this;
    }

    constexpr virtual ~basic_field_traits() noexcept = default;

    [[nodiscard]] constexpr auto name() const noexcept -> std::string_view { return name_; }

private:
    std::string_view name_;
};

template <>
struct field_traits<void> : public ::atom::utils::basic_field_traits {
    using type = void;

    explicit constexpr field_traits() noexcept : basic_field_traits("void") {}
};

template <typename Ty>
requires(!std::is_void_v<Ty>)
struct field_traits<Ty*> : public ::atom::utils::basic_field_traits {
    using type = Ty;

    explicit constexpr field_traits(const char* name, Ty* pointer) noexcept
        : ::atom::utils::basic_field_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr auto get() noexcept -> Ty& { return *pointer_; }
    [[nodiscard]] constexpr auto get() const noexcept -> const Ty& { return *pointer_; }

    [[nodiscard]] constexpr auto pointer() const noexcept -> Ty* { return pointer_; }

private:
    Ty* pointer_;
};

template <typename Ty, typename Class>
struct field_traits<Ty Class::*> : public ::atom::utils::basic_field_traits {
    using type = Ty;

    explicit constexpr field_traits(const char* name, Ty Class::* pointer)
        : ::atom::utils::basic_field_traits(name), pointer_(pointer) {}

    explicit constexpr field_traits(std::string_view name, Ty Class::* pointer)
        : basic_field_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr auto get(Class& instance) const noexcept -> Ty& {
        return instance.*pointer_;
    }
    [[nodiscard]] constexpr auto get(const Class& instance) const noexcept -> const Ty& {
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
    explicit constexpr basic_function_traits(const char* name) noexcept : name_(name) {}
    explicit constexpr basic_function_traits(std::string_view name) noexcept : name_(name) {}

    constexpr basic_function_traits(const basic_function_traits&) noexcept            = default;
    constexpr basic_function_traits& operator=(const basic_function_traits&) noexcept = default;

    constexpr basic_function_traits(basic_function_traits&& obj) noexcept : name_(obj.name_) {}
    constexpr basic_function_traits& operator=(basic_function_traits&& obj) noexcept {
        if (this != &obj) {
            name_ = obj.name_;
        }
        return *this;
    }

    constexpr virtual ~basic_function_traits() noexcept = default;

    [[nodiscard]] constexpr auto name() const noexcept -> std::string_view { return name_; }

private:
    std::string_view name_;
};

template <typename Ret, typename... Args>
struct function_traits<Ret (*)(Args...)> : public ::atom::utils::basic_function_traits {
    using func_type = Ret(Args...);

    explicit constexpr function_traits(const char* name, Ret (*pointer)(Args...)) noexcept
        : ::atom::utils::basic_function_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr std::size_t num_args() const noexcept { return sizeof...(Args); }

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

    explicit constexpr function_traits(const char* name, Ret (Class::*pointer)(Args...)) noexcept
        : ::atom::utils::basic_function_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr std::size_t num_args() const noexcept { return sizeof...(Args); }

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
[[nodiscard]] consteval inline std::string_view name_of() noexcept {
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
[[nodiscard]] consteval inline std::string_view alias_name_of() noexcept {
    return name_of<Ty>();
}

template <concepts::pure Ty>
requires internal::has_alias_name<Ty>
[[nodiscard]] consteval inline std::string_view alias_name_of() noexcept {
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
constexpr inline auto member_count_of_impl() noexcept {
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
consteval auto member_count_of() noexcept {
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

template <concepts::aggregate Ty>
struct outline {
    [[maybe_unused]] static inline std::remove_cvref_t<Ty> value{};
};

template <typename Ty>
FORCE_INLINE constexpr static const std::remove_cvref_t<Ty>& get_object_outline() noexcept {
    return outline<Ty>::value;
}

template <typename Ty>
struct tuple_view_helper;

// Support for type with 0 member.

template <typename Ty>
requires(member_count_v<Ty> == 0)
struct tuple_view_helper<Ty> {
    constexpr static auto tuple_view() noexcept { return std::tuple<>(); }
    constexpr static auto tuple_view(const Ty& obj) noexcept { return std::tuple<>(); }
};

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define TUPLE_VIEW_HELPER(N, ...)                                                                  \
    template <typename Ty>                                                                         \
    requires(member_count_v<Ty> == N)                                                              \
    struct tuple_view_helper<Ty> {                                                                 \
        consteval static auto tuple_view() noexcept {                                              \
            auto& [__VA_ARGS__] = get_object_outline<Ty>();                                        \
            auto tuple          = std::tie(__VA_ARGS__);                                           \
            auto get_ptr = [](auto&... refs) { return std::make_tuple(std::addressof(refs)...); }; \
            return std::apply(get_ptr, tuple);                                                     \
        }                                                                                          \
        constexpr static auto tuple_view(const Ty& obj) noexcept {                                 \
            auto& [__VA_ARGS__] = obj;                                                             \
            return std::tie(__VA_ARGS__);                                                          \
        }                                                                                          \
        constexpr static auto tuple_view(Ty& obj) noexcept {                                       \
            auto& [__VA_ARGS__] = obj;                                                             \
            return std::tie(__VA_ARGS__);                                                          \
        }                                                                                          \
    };                                                                                             \
    //

// NOLINTEND(cppcoreguidelines-macro-usage)

// Support for from 1 to 127

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
constexpr static inline auto struct_to_tuple_view() noexcept {
    return internal::tuple_view_helper<std::remove_cvref_t<Ty>>::tuple_view();
}

template <typename Ty>
constexpr static inline auto object_to_tuple_view(const Ty& obj) noexcept {
    return internal::tuple_view_helper<std::remove_cvref_t<Ty>>::tuple_view(obj);
}

template <typename Ty>
constexpr static inline auto object_to_tuple_view(Ty& obj) noexcept {
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
consteval std::string_view member_name_of() noexcept {
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
FORCE_INLINE constexpr static auto wrap(const Ty& arg) noexcept {
    return wrapper{ arg };
}

} // namespace internal
/*! @endcond */

/**
 * @brief Get members' name of a type.
 *
 */
template <concepts::reflectible Ty>
constexpr inline std::array<std::string_view, member_count_v<Ty>> member_names_of() noexcept {
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

#ifndef ATOM_VECTORIZABLE
    #if defined(__i386__) || defined(__x86_64__)
        #define ATOM_VECTORIZABLE true
    #else
        #define ATOM_VECTORIZABLE false
    #endif
#endif

#if ATOM_VECTORIZABLE
    #include <immintrin.h>
#endif

namespace atom::utils {
/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
FORCE_INLINE constexpr std::size_t hash(std::string_view string) noexcept {
    // DJB2 Hash
    // NOTE: This hash algorithm is not friendly to parallelization support
    // TODO: Another hash algrorithm, which is friendly to parallelization.

#if ATOM_VECTORIZABLE && false
    // bytes
    const size_t parallel_request = 16;

    #if defined(__AVX2__)
    const size_t group_size = 8;
    #elif defined(__SSE2__)
    const size_t group_size = 4;
    #endif
#endif

    const size_t magic_initial_value = 5381;
    const size_t magic               = 5;

    std::size_t value = magic_initial_value;
#if ATOM_VECTORIZABLE && false
    if (string.length() < group_size) {
#endif
        for (const char c : string) {
            value = ((value << magic) + value) + c;
        }
#if ATOM_VECTORIZABLE && false
    }
    else {}
#endif
    return value;
}

} // namespace internal
/*! @endcond */

template <concepts::pure Ty>
consteval size_t hash_of() noexcept {
    constexpr auto name = name_of<Ty>();
    return internal::hash(name);
}

template <concepts::pure Ty>
constexpr inline std::size_t hash_v = hash_of<Ty>;

size_t inline hash_of(std::string_view str) noexcept { return internal::hash(str); }

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
constexpr inline auto& get(Ty&& obj) noexcept {
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
[[nodiscard]] consteval inline auto existance_of() noexcept {
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
[[nodiscard]] constexpr inline auto existance_of(std::string_view name) noexcept -> size_t {
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
[[nodiscard]] consteval inline auto index_of() noexcept -> size_t {
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
[[nodiscard]] constexpr inline auto index_of(std::string_view name) noexcept -> size_t {
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
[[nodiscard]] consteval auto name_of() noexcept {
    constexpr auto count = member_count_of<Ty>;
    static_assert(Index < count, "Index out of range");
    constexpr auto names = member_names_of<Ty>();
    return names[Index];
}

/**
 * @brief Get the name of a member.
 *
 */
template <concepts::reflectible Ty>
[[nodiscard]] constexpr inline auto name_of(size_t index) noexcept {
    constexpr auto names = member_names_of<Ty>();
    return names[index];
}

template <tstring_v Name, typename Ty>
constexpr inline auto& get(Ty& obj) noexcept {
    constexpr auto index = index_of<Name, Ty>();
    static_assert(index < member_count_v<Ty>);
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
consteval auto description_of() noexcept -> description_bits {
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
    const auto mask            = static_cast<description_bits_base>(bits);
    const auto result          = description & mask;
    return result == mask;
}

struct authenticity_params {
    description_bits desc;
    description_bits bits;
};

constexpr inline bool authenticity_of(const authenticity_params params) noexcept {
    auto mask   = static_cast<description_bits_base>(params.bits);
    auto result = static_cast<description_bits_base>(params.desc) & mask;
    return result == mask;
}

} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// module: offset_of
///////////////////////////////////////////////////////////////////////////////

namespace atom::utils {

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
const auto offset_count = 4096;

struct offset_helper {
    offset_helper() = delete;
    std::uint8_t off0;
    std::uint8_t off1;
    std::uint8_t off2;
    std::uint8_t off3;
    std::uint8_t off4;
    std::uint8_t off5;
    std::uint8_t off6;
    std::uint8_t off7;
    std::uint8_t off8;
    std::uint8_t off9;
    std::uint8_t off10;
    std::uint8_t off11;
    std::uint8_t off12;
    std::uint8_t off13;
    std::uint8_t off14;
    std::uint8_t off15;
    std::uint8_t off16;
    std::uint8_t off17;
    std::uint8_t off18;
    std::uint8_t off19;
    std::uint8_t off20;
    std::uint8_t off21;
    std::uint8_t off22;
    std::uint8_t off23;
    std::uint8_t off24;
    std::uint8_t off25;
    std::uint8_t off26;
    std::uint8_t off27;
    std::uint8_t off28;
    std::uint8_t off29;
    std::uint8_t off30;
    std::uint8_t off31;
    std::uint8_t off32;
    std::uint8_t off33;
    std::uint8_t off34;
    std::uint8_t off35;
    std::uint8_t off36;
    std::uint8_t off37;
    std::uint8_t off38;
    std::uint8_t off39;
    std::uint8_t off40;
    std::uint8_t off41;
    std::uint8_t off42;
    std::uint8_t off43;
    std::uint8_t off44;
    std::uint8_t off45;
    std::uint8_t off46;
    std::uint8_t off47;
    std::uint8_t off48;
    std::uint8_t off49;
    std::uint8_t off50;
    std::uint8_t off51;
    std::uint8_t off52;
    std::uint8_t off53;
    std::uint8_t off54;
    std::uint8_t off55;
    std::uint8_t off56;
    std::uint8_t off57;
    std::uint8_t off58;
    std::uint8_t off59;
    std::uint8_t off60;
    std::uint8_t off61;
    std::uint8_t off62;
    std::uint8_t off63;
    std::uint8_t off64;
    std::uint8_t off65;
    std::uint8_t off66;
    std::uint8_t off67;
    std::uint8_t off68;
    std::uint8_t off69;
    std::uint8_t off70;
    std::uint8_t off71;
    std::uint8_t off72;
    std::uint8_t off73;
    std::uint8_t off74;
    std::uint8_t off75;
    std::uint8_t off76;
    std::uint8_t off77;
    std::uint8_t off78;
    std::uint8_t off79;
    std::uint8_t off80;
    std::uint8_t off81;
    std::uint8_t off82;
    std::uint8_t off83;
    std::uint8_t off84;
    std::uint8_t off85;
    std::uint8_t off86;
    std::uint8_t off87;
    std::uint8_t off88;
    std::uint8_t off89;
    std::uint8_t off90;
    std::uint8_t off91;
    std::uint8_t off92;
    std::uint8_t off93;
    std::uint8_t off94;
    std::uint8_t off95;
    std::uint8_t off96;
    std::uint8_t off97;
    std::uint8_t off98;
    std::uint8_t off99;
    std::uint8_t off100;
    std::uint8_t off101;
    std::uint8_t off102;
    std::uint8_t off103;
    std::uint8_t off104;
    std::uint8_t off105;
    std::uint8_t off106;
    std::uint8_t off107;
    std::uint8_t off108;
    std::uint8_t off109;
    std::uint8_t off110;
    std::uint8_t off111;
    std::uint8_t off112;
    std::uint8_t off113;
    std::uint8_t off114;
    std::uint8_t off115;
    std::uint8_t off116;
    std::uint8_t off117;
    std::uint8_t off118;
    std::uint8_t off119;
    std::uint8_t off120;
    std::uint8_t off121;
    std::uint8_t off122;
    std::uint8_t off123;
    std::uint8_t off124;
    std::uint8_t off125;
    std::uint8_t off126;
    std::uint8_t off127;
    std::uint8_t off128;
    std::uint8_t off129;
    std::uint8_t off130;
    std::uint8_t off131;
    std::uint8_t off132;
    std::uint8_t off133;
    std::uint8_t off134;
    std::uint8_t off135;
    std::uint8_t off136;
    std::uint8_t off137;
    std::uint8_t off138;
    std::uint8_t off139;
    std::uint8_t off140;
    std::uint8_t off141;
    std::uint8_t off142;
    std::uint8_t off143;
    std::uint8_t off144;
    std::uint8_t off145;
    std::uint8_t off146;
    std::uint8_t off147;
    std::uint8_t off148;
    std::uint8_t off149;
    std::uint8_t off150;
    std::uint8_t off151;
    std::uint8_t off152;
    std::uint8_t off153;
    std::uint8_t off154;
    std::uint8_t off155;
    std::uint8_t off156;
    std::uint8_t off157;
    std::uint8_t off158;
    std::uint8_t off159;
    std::uint8_t off160;
    std::uint8_t off161;
    std::uint8_t off162;
    std::uint8_t off163;
    std::uint8_t off164;
    std::uint8_t off165;
    std::uint8_t off166;
    std::uint8_t off167;
    std::uint8_t off168;
    std::uint8_t off169;
    std::uint8_t off170;
    std::uint8_t off171;
    std::uint8_t off172;
    std::uint8_t off173;
    std::uint8_t off174;
    std::uint8_t off175;
    std::uint8_t off176;
    std::uint8_t off177;
    std::uint8_t off178;
    std::uint8_t off179;
    std::uint8_t off180;
    std::uint8_t off181;
    std::uint8_t off182;
    std::uint8_t off183;
    std::uint8_t off184;
    std::uint8_t off185;
    std::uint8_t off186;
    std::uint8_t off187;
    std::uint8_t off188;
    std::uint8_t off189;
    std::uint8_t off190;
    std::uint8_t off191;
    std::uint8_t off192;
    std::uint8_t off193;
    std::uint8_t off194;
    std::uint8_t off195;
    std::uint8_t off196;
    std::uint8_t off197;
    std::uint8_t off198;
    std::uint8_t off199;
    std::uint8_t off200;
    std::uint8_t off201;
    std::uint8_t off202;
    std::uint8_t off203;
    std::uint8_t off204;
    std::uint8_t off205;
    std::uint8_t off206;
    std::uint8_t off207;
    std::uint8_t off208;
    std::uint8_t off209;
    std::uint8_t off210;
    std::uint8_t off211;
    std::uint8_t off212;
    std::uint8_t off213;
    std::uint8_t off214;
    std::uint8_t off215;
    std::uint8_t off216;
    std::uint8_t off217;
    std::uint8_t off218;
    std::uint8_t off219;
    std::uint8_t off220;
    std::uint8_t off221;
    std::uint8_t off222;
    std::uint8_t off223;
    std::uint8_t off224;
    std::uint8_t off225;
    std::uint8_t off226;
    std::uint8_t off227;
    std::uint8_t off228;
    std::uint8_t off229;
    std::uint8_t off230;
    std::uint8_t off231;
    std::uint8_t off232;
    std::uint8_t off233;
    std::uint8_t off234;
    std::uint8_t off235;
    std::uint8_t off236;
    std::uint8_t off237;
    std::uint8_t off238;
    std::uint8_t off239;
    std::uint8_t off240;
    std::uint8_t off241;
    std::uint8_t off242;
    std::uint8_t off243;
    std::uint8_t off244;
    std::uint8_t off245;
    std::uint8_t off246;
    std::uint8_t off247;
    std::uint8_t off248;
    std::uint8_t off249;
    std::uint8_t off250;
    std::uint8_t off251;
    std::uint8_t off252;
    std::uint8_t off253;
    std::uint8_t off254;
    std::uint8_t off255;
    std::uint8_t off256;
    std::uint8_t off257;
    std::uint8_t off258;
    std::uint8_t off259;
    std::uint8_t off260;
    std::uint8_t off261;
    std::uint8_t off262;
    std::uint8_t off263;
    std::uint8_t off264;
    std::uint8_t off265;
    std::uint8_t off266;
    std::uint8_t off267;
    std::uint8_t off268;
    std::uint8_t off269;
    std::uint8_t off270;
    std::uint8_t off271;
    std::uint8_t off272;
    std::uint8_t off273;
    std::uint8_t off274;
    std::uint8_t off275;
    std::uint8_t off276;
    std::uint8_t off277;
    std::uint8_t off278;
    std::uint8_t off279;
    std::uint8_t off280;
    std::uint8_t off281;
    std::uint8_t off282;
    std::uint8_t off283;
    std::uint8_t off284;
    std::uint8_t off285;
    std::uint8_t off286;
    std::uint8_t off287;
    std::uint8_t off288;
    std::uint8_t off289;
    std::uint8_t off290;
    std::uint8_t off291;
    std::uint8_t off292;
    std::uint8_t off293;
    std::uint8_t off294;
    std::uint8_t off295;
    std::uint8_t off296;
    std::uint8_t off297;
    std::uint8_t off298;
    std::uint8_t off299;
    std::uint8_t off300;
    std::uint8_t off301;
    std::uint8_t off302;
    std::uint8_t off303;
    std::uint8_t off304;
    std::uint8_t off305;
    std::uint8_t off306;
    std::uint8_t off307;
    std::uint8_t off308;
    std::uint8_t off309;
    std::uint8_t off310;
    std::uint8_t off311;
    std::uint8_t off312;
    std::uint8_t off313;
    std::uint8_t off314;
    std::uint8_t off315;
    std::uint8_t off316;
    std::uint8_t off317;
    std::uint8_t off318;
    std::uint8_t off319;
    std::uint8_t off320;
    std::uint8_t off321;
    std::uint8_t off322;
    std::uint8_t off323;
    std::uint8_t off324;
    std::uint8_t off325;
    std::uint8_t off326;
    std::uint8_t off327;
    std::uint8_t off328;
    std::uint8_t off329;
    std::uint8_t off330;
    std::uint8_t off331;
    std::uint8_t off332;
    std::uint8_t off333;
    std::uint8_t off334;
    std::uint8_t off335;
    std::uint8_t off336;
    std::uint8_t off337;
    std::uint8_t off338;
    std::uint8_t off339;
    std::uint8_t off340;
    std::uint8_t off341;
    std::uint8_t off342;
    std::uint8_t off343;
    std::uint8_t off344;
    std::uint8_t off345;
    std::uint8_t off346;
    std::uint8_t off347;
    std::uint8_t off348;
    std::uint8_t off349;
    std::uint8_t off350;
    std::uint8_t off351;
    std::uint8_t off352;
    std::uint8_t off353;
    std::uint8_t off354;
    std::uint8_t off355;
    std::uint8_t off356;
    std::uint8_t off357;
    std::uint8_t off358;
    std::uint8_t off359;
    std::uint8_t off360;
    std::uint8_t off361;
    std::uint8_t off362;
    std::uint8_t off363;
    std::uint8_t off364;
    std::uint8_t off365;
    std::uint8_t off366;
    std::uint8_t off367;
    std::uint8_t off368;
    std::uint8_t off369;
    std::uint8_t off370;
    std::uint8_t off371;
    std::uint8_t off372;
    std::uint8_t off373;
    std::uint8_t off374;
    std::uint8_t off375;
    std::uint8_t off376;
    std::uint8_t off377;
    std::uint8_t off378;
    std::uint8_t off379;
    std::uint8_t off380;
    std::uint8_t off381;
    std::uint8_t off382;
    std::uint8_t off383;
    std::uint8_t off384;
    std::uint8_t off385;
    std::uint8_t off386;
    std::uint8_t off387;
    std::uint8_t off388;
    std::uint8_t off389;
    std::uint8_t off390;
    std::uint8_t off391;
    std::uint8_t off392;
    std::uint8_t off393;
    std::uint8_t off394;
    std::uint8_t off395;
    std::uint8_t off396;
    std::uint8_t off397;
    std::uint8_t off398;
    std::uint8_t off399;
    std::uint8_t off400;
    std::uint8_t off401;
    std::uint8_t off402;
    std::uint8_t off403;
    std::uint8_t off404;
    std::uint8_t off405;
    std::uint8_t off406;
    std::uint8_t off407;
    std::uint8_t off408;
    std::uint8_t off409;
    std::uint8_t off410;
    std::uint8_t off411;
    std::uint8_t off412;
    std::uint8_t off413;
    std::uint8_t off414;
    std::uint8_t off415;
    std::uint8_t off416;
    std::uint8_t off417;
    std::uint8_t off418;
    std::uint8_t off419;
    std::uint8_t off420;
    std::uint8_t off421;
    std::uint8_t off422;
    std::uint8_t off423;
    std::uint8_t off424;
    std::uint8_t off425;
    std::uint8_t off426;
    std::uint8_t off427;
    std::uint8_t off428;
    std::uint8_t off429;
    std::uint8_t off430;
    std::uint8_t off431;
    std::uint8_t off432;
    std::uint8_t off433;
    std::uint8_t off434;
    std::uint8_t off435;
    std::uint8_t off436;
    std::uint8_t off437;
    std::uint8_t off438;
    std::uint8_t off439;
    std::uint8_t off440;
    std::uint8_t off441;
    std::uint8_t off442;
    std::uint8_t off443;
    std::uint8_t off444;
    std::uint8_t off445;
    std::uint8_t off446;
    std::uint8_t off447;
    std::uint8_t off448;
    std::uint8_t off449;
    std::uint8_t off450;
    std::uint8_t off451;
    std::uint8_t off452;
    std::uint8_t off453;
    std::uint8_t off454;
    std::uint8_t off455;
    std::uint8_t off456;
    std::uint8_t off457;
    std::uint8_t off458;
    std::uint8_t off459;
    std::uint8_t off460;
    std::uint8_t off461;
    std::uint8_t off462;
    std::uint8_t off463;
    std::uint8_t off464;
    std::uint8_t off465;
    std::uint8_t off466;
    std::uint8_t off467;
    std::uint8_t off468;
    std::uint8_t off469;
    std::uint8_t off470;
    std::uint8_t off471;
    std::uint8_t off472;
    std::uint8_t off473;
    std::uint8_t off474;
    std::uint8_t off475;
    std::uint8_t off476;
    std::uint8_t off477;
    std::uint8_t off478;
    std::uint8_t off479;
    std::uint8_t off480;
    std::uint8_t off481;
    std::uint8_t off482;
    std::uint8_t off483;
    std::uint8_t off484;
    std::uint8_t off485;
    std::uint8_t off486;
    std::uint8_t off487;
    std::uint8_t off488;
    std::uint8_t off489;
    std::uint8_t off490;
    std::uint8_t off491;
    std::uint8_t off492;
    std::uint8_t off493;
    std::uint8_t off494;
    std::uint8_t off495;
    std::uint8_t off496;
    std::uint8_t off497;
    std::uint8_t off498;
    std::uint8_t off499;
    std::uint8_t off500;
    std::uint8_t off501;
    std::uint8_t off502;
    std::uint8_t off503;
    std::uint8_t off504;
    std::uint8_t off505;
    std::uint8_t off506;
    std::uint8_t off507;
    std::uint8_t off508;
    std::uint8_t off509;
    std::uint8_t off510;
    std::uint8_t off511;
    std::uint8_t off512;
    std::uint8_t off513;
    std::uint8_t off514;
    std::uint8_t off515;
    std::uint8_t off516;
    std::uint8_t off517;
    std::uint8_t off518;
    std::uint8_t off519;
    std::uint8_t off520;
    std::uint8_t off521;
    std::uint8_t off522;
    std::uint8_t off523;
    std::uint8_t off524;
    std::uint8_t off525;
    std::uint8_t off526;
    std::uint8_t off527;
    std::uint8_t off528;
    std::uint8_t off529;
    std::uint8_t off530;
    std::uint8_t off531;
    std::uint8_t off532;
    std::uint8_t off533;
    std::uint8_t off534;
    std::uint8_t off535;
    std::uint8_t off536;
    std::uint8_t off537;
    std::uint8_t off538;
    std::uint8_t off539;
    std::uint8_t off540;
    std::uint8_t off541;
    std::uint8_t off542;
    std::uint8_t off543;
    std::uint8_t off544;
    std::uint8_t off545;
    std::uint8_t off546;
    std::uint8_t off547;
    std::uint8_t off548;
    std::uint8_t off549;
    std::uint8_t off550;
    std::uint8_t off551;
    std::uint8_t off552;
    std::uint8_t off553;
    std::uint8_t off554;
    std::uint8_t off555;
    std::uint8_t off556;
    std::uint8_t off557;
    std::uint8_t off558;
    std::uint8_t off559;
    std::uint8_t off560;
    std::uint8_t off561;
    std::uint8_t off562;
    std::uint8_t off563;
    std::uint8_t off564;
    std::uint8_t off565;
    std::uint8_t off566;
    std::uint8_t off567;
    std::uint8_t off568;
    std::uint8_t off569;
    std::uint8_t off570;
    std::uint8_t off571;
    std::uint8_t off572;
    std::uint8_t off573;
    std::uint8_t off574;
    std::uint8_t off575;
    std::uint8_t off576;
    std::uint8_t off577;
    std::uint8_t off578;
    std::uint8_t off579;
    std::uint8_t off580;
    std::uint8_t off581;
    std::uint8_t off582;
    std::uint8_t off583;
    std::uint8_t off584;
    std::uint8_t off585;
    std::uint8_t off586;
    std::uint8_t off587;
    std::uint8_t off588;
    std::uint8_t off589;
    std::uint8_t off590;
    std::uint8_t off591;
    std::uint8_t off592;
    std::uint8_t off593;
    std::uint8_t off594;
    std::uint8_t off595;
    std::uint8_t off596;
    std::uint8_t off597;
    std::uint8_t off598;
    std::uint8_t off599;
    std::uint8_t off600;
    std::uint8_t off601;
    std::uint8_t off602;
    std::uint8_t off603;
    std::uint8_t off604;
    std::uint8_t off605;
    std::uint8_t off606;
    std::uint8_t off607;
    std::uint8_t off608;
    std::uint8_t off609;
    std::uint8_t off610;
    std::uint8_t off611;
    std::uint8_t off612;
    std::uint8_t off613;
    std::uint8_t off614;
    std::uint8_t off615;
    std::uint8_t off616;
    std::uint8_t off617;
    std::uint8_t off618;
    std::uint8_t off619;
    std::uint8_t off620;
    std::uint8_t off621;
    std::uint8_t off622;
    std::uint8_t off623;
    std::uint8_t off624;
    std::uint8_t off625;
    std::uint8_t off626;
    std::uint8_t off627;
    std::uint8_t off628;
    std::uint8_t off629;
    std::uint8_t off630;
    std::uint8_t off631;
    std::uint8_t off632;
    std::uint8_t off633;
    std::uint8_t off634;
    std::uint8_t off635;
    std::uint8_t off636;
    std::uint8_t off637;
    std::uint8_t off638;
    std::uint8_t off639;
    std::uint8_t off640;
    std::uint8_t off641;
    std::uint8_t off642;
    std::uint8_t off643;
    std::uint8_t off644;
    std::uint8_t off645;
    std::uint8_t off646;
    std::uint8_t off647;
    std::uint8_t off648;
    std::uint8_t off649;
    std::uint8_t off650;
    std::uint8_t off651;
    std::uint8_t off652;
    std::uint8_t off653;
    std::uint8_t off654;
    std::uint8_t off655;
    std::uint8_t off656;
    std::uint8_t off657;
    std::uint8_t off658;
    std::uint8_t off659;
    std::uint8_t off660;
    std::uint8_t off661;
    std::uint8_t off662;
    std::uint8_t off663;
    std::uint8_t off664;
    std::uint8_t off665;
    std::uint8_t off666;
    std::uint8_t off667;
    std::uint8_t off668;
    std::uint8_t off669;
    std::uint8_t off670;
    std::uint8_t off671;
    std::uint8_t off672;
    std::uint8_t off673;
    std::uint8_t off674;
    std::uint8_t off675;
    std::uint8_t off676;
    std::uint8_t off677;
    std::uint8_t off678;
    std::uint8_t off679;
    std::uint8_t off680;
    std::uint8_t off681;
    std::uint8_t off682;
    std::uint8_t off683;
    std::uint8_t off684;
    std::uint8_t off685;
    std::uint8_t off686;
    std::uint8_t off687;
    std::uint8_t off688;
    std::uint8_t off689;
    std::uint8_t off690;
    std::uint8_t off691;
    std::uint8_t off692;
    std::uint8_t off693;
    std::uint8_t off694;
    std::uint8_t off695;
    std::uint8_t off696;
    std::uint8_t off697;
    std::uint8_t off698;
    std::uint8_t off699;
    std::uint8_t off700;
    std::uint8_t off701;
    std::uint8_t off702;
    std::uint8_t off703;
    std::uint8_t off704;
    std::uint8_t off705;
    std::uint8_t off706;
    std::uint8_t off707;
    std::uint8_t off708;
    std::uint8_t off709;
    std::uint8_t off710;
    std::uint8_t off711;
    std::uint8_t off712;
    std::uint8_t off713;
    std::uint8_t off714;
    std::uint8_t off715;
    std::uint8_t off716;
    std::uint8_t off717;
    std::uint8_t off718;
    std::uint8_t off719;
    std::uint8_t off720;
    std::uint8_t off721;
    std::uint8_t off722;
    std::uint8_t off723;
    std::uint8_t off724;
    std::uint8_t off725;
    std::uint8_t off726;
    std::uint8_t off727;
    std::uint8_t off728;
    std::uint8_t off729;
    std::uint8_t off730;
    std::uint8_t off731;
    std::uint8_t off732;
    std::uint8_t off733;
    std::uint8_t off734;
    std::uint8_t off735;
    std::uint8_t off736;
    std::uint8_t off737;
    std::uint8_t off738;
    std::uint8_t off739;
    std::uint8_t off740;
    std::uint8_t off741;
    std::uint8_t off742;
    std::uint8_t off743;
    std::uint8_t off744;
    std::uint8_t off745;
    std::uint8_t off746;
    std::uint8_t off747;
    std::uint8_t off748;
    std::uint8_t off749;
    std::uint8_t off750;
    std::uint8_t off751;
    std::uint8_t off752;
    std::uint8_t off753;
    std::uint8_t off754;
    std::uint8_t off755;
    std::uint8_t off756;
    std::uint8_t off757;
    std::uint8_t off758;
    std::uint8_t off759;
    std::uint8_t off760;
    std::uint8_t off761;
    std::uint8_t off762;
    std::uint8_t off763;
    std::uint8_t off764;
    std::uint8_t off765;
    std::uint8_t off766;
    std::uint8_t off767;
    std::uint8_t off768;
    std::uint8_t off769;
    std::uint8_t off770;
    std::uint8_t off771;
    std::uint8_t off772;
    std::uint8_t off773;
    std::uint8_t off774;
    std::uint8_t off775;
    std::uint8_t off776;
    std::uint8_t off777;
    std::uint8_t off778;
    std::uint8_t off779;
    std::uint8_t off780;
    std::uint8_t off781;
    std::uint8_t off782;
    std::uint8_t off783;
    std::uint8_t off784;
    std::uint8_t off785;
    std::uint8_t off786;
    std::uint8_t off787;
    std::uint8_t off788;
    std::uint8_t off789;
    std::uint8_t off790;
    std::uint8_t off791;
    std::uint8_t off792;
    std::uint8_t off793;
    std::uint8_t off794;
    std::uint8_t off795;
    std::uint8_t off796;
    std::uint8_t off797;
    std::uint8_t off798;
    std::uint8_t off799;
    std::uint8_t off800;
    std::uint8_t off801;
    std::uint8_t off802;
    std::uint8_t off803;
    std::uint8_t off804;
    std::uint8_t off805;
    std::uint8_t off806;
    std::uint8_t off807;
    std::uint8_t off808;
    std::uint8_t off809;
    std::uint8_t off810;
    std::uint8_t off811;
    std::uint8_t off812;
    std::uint8_t off813;
    std::uint8_t off814;
    std::uint8_t off815;
    std::uint8_t off816;
    std::uint8_t off817;
    std::uint8_t off818;
    std::uint8_t off819;
    std::uint8_t off820;
    std::uint8_t off821;
    std::uint8_t off822;
    std::uint8_t off823;
    std::uint8_t off824;
    std::uint8_t off825;
    std::uint8_t off826;
    std::uint8_t off827;
    std::uint8_t off828;
    std::uint8_t off829;
    std::uint8_t off830;
    std::uint8_t off831;
    std::uint8_t off832;
    std::uint8_t off833;
    std::uint8_t off834;
    std::uint8_t off835;
    std::uint8_t off836;
    std::uint8_t off837;
    std::uint8_t off838;
    std::uint8_t off839;
    std::uint8_t off840;
    std::uint8_t off841;
    std::uint8_t off842;
    std::uint8_t off843;
    std::uint8_t off844;
    std::uint8_t off845;
    std::uint8_t off846;
    std::uint8_t off847;
    std::uint8_t off848;
    std::uint8_t off849;
    std::uint8_t off850;
    std::uint8_t off851;
    std::uint8_t off852;
    std::uint8_t off853;
    std::uint8_t off854;
    std::uint8_t off855;
    std::uint8_t off856;
    std::uint8_t off857;
    std::uint8_t off858;
    std::uint8_t off859;
    std::uint8_t off860;
    std::uint8_t off861;
    std::uint8_t off862;
    std::uint8_t off863;
    std::uint8_t off864;
    std::uint8_t off865;
    std::uint8_t off866;
    std::uint8_t off867;
    std::uint8_t off868;
    std::uint8_t off869;
    std::uint8_t off870;
    std::uint8_t off871;
    std::uint8_t off872;
    std::uint8_t off873;
    std::uint8_t off874;
    std::uint8_t off875;
    std::uint8_t off876;
    std::uint8_t off877;
    std::uint8_t off878;
    std::uint8_t off879;
    std::uint8_t off880;
    std::uint8_t off881;
    std::uint8_t off882;
    std::uint8_t off883;
    std::uint8_t off884;
    std::uint8_t off885;
    std::uint8_t off886;
    std::uint8_t off887;
    std::uint8_t off888;
    std::uint8_t off889;
    std::uint8_t off890;
    std::uint8_t off891;
    std::uint8_t off892;
    std::uint8_t off893;
    std::uint8_t off894;
    std::uint8_t off895;
    std::uint8_t off896;
    std::uint8_t off897;
    std::uint8_t off898;
    std::uint8_t off899;
    std::uint8_t off900;
    std::uint8_t off901;
    std::uint8_t off902;
    std::uint8_t off903;
    std::uint8_t off904;
    std::uint8_t off905;
    std::uint8_t off906;
    std::uint8_t off907;
    std::uint8_t off908;
    std::uint8_t off909;
    std::uint8_t off910;
    std::uint8_t off911;
    std::uint8_t off912;
    std::uint8_t off913;
    std::uint8_t off914;
    std::uint8_t off915;
    std::uint8_t off916;
    std::uint8_t off917;
    std::uint8_t off918;
    std::uint8_t off919;
    std::uint8_t off920;
    std::uint8_t off921;
    std::uint8_t off922;
    std::uint8_t off923;
    std::uint8_t off924;
    std::uint8_t off925;
    std::uint8_t off926;
    std::uint8_t off927;
    std::uint8_t off928;
    std::uint8_t off929;
    std::uint8_t off930;
    std::uint8_t off931;
    std::uint8_t off932;
    std::uint8_t off933;
    std::uint8_t off934;
    std::uint8_t off935;
    std::uint8_t off936;
    std::uint8_t off937;
    std::uint8_t off938;
    std::uint8_t off939;
    std::uint8_t off940;
    std::uint8_t off941;
    std::uint8_t off942;
    std::uint8_t off943;
    std::uint8_t off944;
    std::uint8_t off945;
    std::uint8_t off946;
    std::uint8_t off947;
    std::uint8_t off948;
    std::uint8_t off949;
    std::uint8_t off950;
    std::uint8_t off951;
    std::uint8_t off952;
    std::uint8_t off953;
    std::uint8_t off954;
    std::uint8_t off955;
    std::uint8_t off956;
    std::uint8_t off957;
    std::uint8_t off958;
    std::uint8_t off959;
    std::uint8_t off960;
    std::uint8_t off961;
    std::uint8_t off962;
    std::uint8_t off963;
    std::uint8_t off964;
    std::uint8_t off965;
    std::uint8_t off966;
    std::uint8_t off967;
    std::uint8_t off968;
    std::uint8_t off969;
    std::uint8_t off970;
    std::uint8_t off971;
    std::uint8_t off972;
    std::uint8_t off973;
    std::uint8_t off974;
    std::uint8_t off975;
    std::uint8_t off976;
    std::uint8_t off977;
    std::uint8_t off978;
    std::uint8_t off979;
    std::uint8_t off980;
    std::uint8_t off981;
    std::uint8_t off982;
    std::uint8_t off983;
    std::uint8_t off984;
    std::uint8_t off985;
    std::uint8_t off986;
    std::uint8_t off987;
    std::uint8_t off988;
    std::uint8_t off989;
    std::uint8_t off990;
    std::uint8_t off991;
    std::uint8_t off992;
    std::uint8_t off993;
    std::uint8_t off994;
    std::uint8_t off995;
    std::uint8_t off996;
    std::uint8_t off997;
    std::uint8_t off998;
    std::uint8_t off999;
    std::uint8_t off1000;
    std::uint8_t off1001;
    std::uint8_t off1002;
    std::uint8_t off1003;
    std::uint8_t off1004;
    std::uint8_t off1005;
    std::uint8_t off1006;
    std::uint8_t off1007;
    std::uint8_t off1008;
    std::uint8_t off1009;
    std::uint8_t off1010;
    std::uint8_t off1011;
    std::uint8_t off1012;
    std::uint8_t off1013;
    std::uint8_t off1014;
    std::uint8_t off1015;
    std::uint8_t off1016;
    std::uint8_t off1017;
    std::uint8_t off1018;
    std::uint8_t off1019;
    std::uint8_t off1020;
    std::uint8_t off1021;
    std::uint8_t off1022;
    std::uint8_t off1023;
    std::uint8_t off1024;
    std::uint8_t off1025;
    std::uint8_t off1026;
    std::uint8_t off1027;
    std::uint8_t off1028;
    std::uint8_t off1029;
    std::uint8_t off1030;
    std::uint8_t off1031;
    std::uint8_t off1032;
    std::uint8_t off1033;
    std::uint8_t off1034;
    std::uint8_t off1035;
    std::uint8_t off1036;
    std::uint8_t off1037;
    std::uint8_t off1038;
    std::uint8_t off1039;
    std::uint8_t off1040;
    std::uint8_t off1041;
    std::uint8_t off1042;
    std::uint8_t off1043;
    std::uint8_t off1044;
    std::uint8_t off1045;
    std::uint8_t off1046;
    std::uint8_t off1047;
    std::uint8_t off1048;
    std::uint8_t off1049;
    std::uint8_t off1050;
    std::uint8_t off1051;
    std::uint8_t off1052;
    std::uint8_t off1053;
    std::uint8_t off1054;
    std::uint8_t off1055;
    std::uint8_t off1056;
    std::uint8_t off1057;
    std::uint8_t off1058;
    std::uint8_t off1059;
    std::uint8_t off1060;
    std::uint8_t off1061;
    std::uint8_t off1062;
    std::uint8_t off1063;
    std::uint8_t off1064;
    std::uint8_t off1065;
    std::uint8_t off1066;
    std::uint8_t off1067;
    std::uint8_t off1068;
    std::uint8_t off1069;
    std::uint8_t off1070;
    std::uint8_t off1071;
    std::uint8_t off1072;
    std::uint8_t off1073;
    std::uint8_t off1074;
    std::uint8_t off1075;
    std::uint8_t off1076;
    std::uint8_t off1077;
    std::uint8_t off1078;
    std::uint8_t off1079;
    std::uint8_t off1080;
    std::uint8_t off1081;
    std::uint8_t off1082;
    std::uint8_t off1083;
    std::uint8_t off1084;
    std::uint8_t off1085;
    std::uint8_t off1086;
    std::uint8_t off1087;
    std::uint8_t off1088;
    std::uint8_t off1089;
    std::uint8_t off1090;
    std::uint8_t off1091;
    std::uint8_t off1092;
    std::uint8_t off1093;
    std::uint8_t off1094;
    std::uint8_t off1095;
    std::uint8_t off1096;
    std::uint8_t off1097;
    std::uint8_t off1098;
    std::uint8_t off1099;
    std::uint8_t off1100;
    std::uint8_t off1101;
    std::uint8_t off1102;
    std::uint8_t off1103;
    std::uint8_t off1104;
    std::uint8_t off1105;
    std::uint8_t off1106;
    std::uint8_t off1107;
    std::uint8_t off1108;
    std::uint8_t off1109;
    std::uint8_t off1110;
    std::uint8_t off1111;
    std::uint8_t off1112;
    std::uint8_t off1113;
    std::uint8_t off1114;
    std::uint8_t off1115;
    std::uint8_t off1116;
    std::uint8_t off1117;
    std::uint8_t off1118;
    std::uint8_t off1119;
    std::uint8_t off1120;
    std::uint8_t off1121;
    std::uint8_t off1122;
    std::uint8_t off1123;
    std::uint8_t off1124;
    std::uint8_t off1125;
    std::uint8_t off1126;
    std::uint8_t off1127;
    std::uint8_t off1128;
    std::uint8_t off1129;
    std::uint8_t off1130;
    std::uint8_t off1131;
    std::uint8_t off1132;
    std::uint8_t off1133;
    std::uint8_t off1134;
    std::uint8_t off1135;
    std::uint8_t off1136;
    std::uint8_t off1137;
    std::uint8_t off1138;
    std::uint8_t off1139;
    std::uint8_t off1140;
    std::uint8_t off1141;
    std::uint8_t off1142;
    std::uint8_t off1143;
    std::uint8_t off1144;
    std::uint8_t off1145;
    std::uint8_t off1146;
    std::uint8_t off1147;
    std::uint8_t off1148;
    std::uint8_t off1149;
    std::uint8_t off1150;
    std::uint8_t off1151;
    std::uint8_t off1152;
    std::uint8_t off1153;
    std::uint8_t off1154;
    std::uint8_t off1155;
    std::uint8_t off1156;
    std::uint8_t off1157;
    std::uint8_t off1158;
    std::uint8_t off1159;
    std::uint8_t off1160;
    std::uint8_t off1161;
    std::uint8_t off1162;
    std::uint8_t off1163;
    std::uint8_t off1164;
    std::uint8_t off1165;
    std::uint8_t off1166;
    std::uint8_t off1167;
    std::uint8_t off1168;
    std::uint8_t off1169;
    std::uint8_t off1170;
    std::uint8_t off1171;
    std::uint8_t off1172;
    std::uint8_t off1173;
    std::uint8_t off1174;
    std::uint8_t off1175;
    std::uint8_t off1176;
    std::uint8_t off1177;
    std::uint8_t off1178;
    std::uint8_t off1179;
    std::uint8_t off1180;
    std::uint8_t off1181;
    std::uint8_t off1182;
    std::uint8_t off1183;
    std::uint8_t off1184;
    std::uint8_t off1185;
    std::uint8_t off1186;
    std::uint8_t off1187;
    std::uint8_t off1188;
    std::uint8_t off1189;
    std::uint8_t off1190;
    std::uint8_t off1191;
    std::uint8_t off1192;
    std::uint8_t off1193;
    std::uint8_t off1194;
    std::uint8_t off1195;
    std::uint8_t off1196;
    std::uint8_t off1197;
    std::uint8_t off1198;
    std::uint8_t off1199;
    std::uint8_t off1200;
    std::uint8_t off1201;
    std::uint8_t off1202;
    std::uint8_t off1203;
    std::uint8_t off1204;
    std::uint8_t off1205;
    std::uint8_t off1206;
    std::uint8_t off1207;
    std::uint8_t off1208;
    std::uint8_t off1209;
    std::uint8_t off1210;
    std::uint8_t off1211;
    std::uint8_t off1212;
    std::uint8_t off1213;
    std::uint8_t off1214;
    std::uint8_t off1215;
    std::uint8_t off1216;
    std::uint8_t off1217;
    std::uint8_t off1218;
    std::uint8_t off1219;
    std::uint8_t off1220;
    std::uint8_t off1221;
    std::uint8_t off1222;
    std::uint8_t off1223;
    std::uint8_t off1224;
    std::uint8_t off1225;
    std::uint8_t off1226;
    std::uint8_t off1227;
    std::uint8_t off1228;
    std::uint8_t off1229;
    std::uint8_t off1230;
    std::uint8_t off1231;
    std::uint8_t off1232;
    std::uint8_t off1233;
    std::uint8_t off1234;
    std::uint8_t off1235;
    std::uint8_t off1236;
    std::uint8_t off1237;
    std::uint8_t off1238;
    std::uint8_t off1239;
    std::uint8_t off1240;
    std::uint8_t off1241;
    std::uint8_t off1242;
    std::uint8_t off1243;
    std::uint8_t off1244;
    std::uint8_t off1245;
    std::uint8_t off1246;
    std::uint8_t off1247;
    std::uint8_t off1248;
    std::uint8_t off1249;
    std::uint8_t off1250;
    std::uint8_t off1251;
    std::uint8_t off1252;
    std::uint8_t off1253;
    std::uint8_t off1254;
    std::uint8_t off1255;
    std::uint8_t off1256;
    std::uint8_t off1257;
    std::uint8_t off1258;
    std::uint8_t off1259;
    std::uint8_t off1260;
    std::uint8_t off1261;
    std::uint8_t off1262;
    std::uint8_t off1263;
    std::uint8_t off1264;
    std::uint8_t off1265;
    std::uint8_t off1266;
    std::uint8_t off1267;
    std::uint8_t off1268;
    std::uint8_t off1269;
    std::uint8_t off1270;
    std::uint8_t off1271;
    std::uint8_t off1272;
    std::uint8_t off1273;
    std::uint8_t off1274;
    std::uint8_t off1275;
    std::uint8_t off1276;
    std::uint8_t off1277;
    std::uint8_t off1278;
    std::uint8_t off1279;
    std::uint8_t off1280;
    std::uint8_t off1281;
    std::uint8_t off1282;
    std::uint8_t off1283;
    std::uint8_t off1284;
    std::uint8_t off1285;
    std::uint8_t off1286;
    std::uint8_t off1287;
    std::uint8_t off1288;
    std::uint8_t off1289;
    std::uint8_t off1290;
    std::uint8_t off1291;
    std::uint8_t off1292;
    std::uint8_t off1293;
    std::uint8_t off1294;
    std::uint8_t off1295;
    std::uint8_t off1296;
    std::uint8_t off1297;
    std::uint8_t off1298;
    std::uint8_t off1299;
    std::uint8_t off1300;
    std::uint8_t off1301;
    std::uint8_t off1302;
    std::uint8_t off1303;
    std::uint8_t off1304;
    std::uint8_t off1305;
    std::uint8_t off1306;
    std::uint8_t off1307;
    std::uint8_t off1308;
    std::uint8_t off1309;
    std::uint8_t off1310;
    std::uint8_t off1311;
    std::uint8_t off1312;
    std::uint8_t off1313;
    std::uint8_t off1314;
    std::uint8_t off1315;
    std::uint8_t off1316;
    std::uint8_t off1317;
    std::uint8_t off1318;
    std::uint8_t off1319;
    std::uint8_t off1320;
    std::uint8_t off1321;
    std::uint8_t off1322;
    std::uint8_t off1323;
    std::uint8_t off1324;
    std::uint8_t off1325;
    std::uint8_t off1326;
    std::uint8_t off1327;
    std::uint8_t off1328;
    std::uint8_t off1329;
    std::uint8_t off1330;
    std::uint8_t off1331;
    std::uint8_t off1332;
    std::uint8_t off1333;
    std::uint8_t off1334;
    std::uint8_t off1335;
    std::uint8_t off1336;
    std::uint8_t off1337;
    std::uint8_t off1338;
    std::uint8_t off1339;
    std::uint8_t off1340;
    std::uint8_t off1341;
    std::uint8_t off1342;
    std::uint8_t off1343;
    std::uint8_t off1344;
    std::uint8_t off1345;
    std::uint8_t off1346;
    std::uint8_t off1347;
    std::uint8_t off1348;
    std::uint8_t off1349;
    std::uint8_t off1350;
    std::uint8_t off1351;
    std::uint8_t off1352;
    std::uint8_t off1353;
    std::uint8_t off1354;
    std::uint8_t off1355;
    std::uint8_t off1356;
    std::uint8_t off1357;
    std::uint8_t off1358;
    std::uint8_t off1359;
    std::uint8_t off1360;
    std::uint8_t off1361;
    std::uint8_t off1362;
    std::uint8_t off1363;
    std::uint8_t off1364;
    std::uint8_t off1365;
    std::uint8_t off1366;
    std::uint8_t off1367;
    std::uint8_t off1368;
    std::uint8_t off1369;
    std::uint8_t off1370;
    std::uint8_t off1371;
    std::uint8_t off1372;
    std::uint8_t off1373;
    std::uint8_t off1374;
    std::uint8_t off1375;
    std::uint8_t off1376;
    std::uint8_t off1377;
    std::uint8_t off1378;
    std::uint8_t off1379;
    std::uint8_t off1380;
    std::uint8_t off1381;
    std::uint8_t off1382;
    std::uint8_t off1383;
    std::uint8_t off1384;
    std::uint8_t off1385;
    std::uint8_t off1386;
    std::uint8_t off1387;
    std::uint8_t off1388;
    std::uint8_t off1389;
    std::uint8_t off1390;
    std::uint8_t off1391;
    std::uint8_t off1392;
    std::uint8_t off1393;
    std::uint8_t off1394;
    std::uint8_t off1395;
    std::uint8_t off1396;
    std::uint8_t off1397;
    std::uint8_t off1398;
    std::uint8_t off1399;
    std::uint8_t off1400;
    std::uint8_t off1401;
    std::uint8_t off1402;
    std::uint8_t off1403;
    std::uint8_t off1404;
    std::uint8_t off1405;
    std::uint8_t off1406;
    std::uint8_t off1407;
    std::uint8_t off1408;
    std::uint8_t off1409;
    std::uint8_t off1410;
    std::uint8_t off1411;
    std::uint8_t off1412;
    std::uint8_t off1413;
    std::uint8_t off1414;
    std::uint8_t off1415;
    std::uint8_t off1416;
    std::uint8_t off1417;
    std::uint8_t off1418;
    std::uint8_t off1419;
    std::uint8_t off1420;
    std::uint8_t off1421;
    std::uint8_t off1422;
    std::uint8_t off1423;
    std::uint8_t off1424;
    std::uint8_t off1425;
    std::uint8_t off1426;
    std::uint8_t off1427;
    std::uint8_t off1428;
    std::uint8_t off1429;
    std::uint8_t off1430;
    std::uint8_t off1431;
    std::uint8_t off1432;
    std::uint8_t off1433;
    std::uint8_t off1434;
    std::uint8_t off1435;
    std::uint8_t off1436;
    std::uint8_t off1437;
    std::uint8_t off1438;
    std::uint8_t off1439;
    std::uint8_t off1440;
    std::uint8_t off1441;
    std::uint8_t off1442;
    std::uint8_t off1443;
    std::uint8_t off1444;
    std::uint8_t off1445;
    std::uint8_t off1446;
    std::uint8_t off1447;
    std::uint8_t off1448;
    std::uint8_t off1449;
    std::uint8_t off1450;
    std::uint8_t off1451;
    std::uint8_t off1452;
    std::uint8_t off1453;
    std::uint8_t off1454;
    std::uint8_t off1455;
    std::uint8_t off1456;
    std::uint8_t off1457;
    std::uint8_t off1458;
    std::uint8_t off1459;
    std::uint8_t off1460;
    std::uint8_t off1461;
    std::uint8_t off1462;
    std::uint8_t off1463;
    std::uint8_t off1464;
    std::uint8_t off1465;
    std::uint8_t off1466;
    std::uint8_t off1467;
    std::uint8_t off1468;
    std::uint8_t off1469;
    std::uint8_t off1470;
    std::uint8_t off1471;
    std::uint8_t off1472;
    std::uint8_t off1473;
    std::uint8_t off1474;
    std::uint8_t off1475;
    std::uint8_t off1476;
    std::uint8_t off1477;
    std::uint8_t off1478;
    std::uint8_t off1479;
    std::uint8_t off1480;
    std::uint8_t off1481;
    std::uint8_t off1482;
    std::uint8_t off1483;
    std::uint8_t off1484;
    std::uint8_t off1485;
    std::uint8_t off1486;
    std::uint8_t off1487;
    std::uint8_t off1488;
    std::uint8_t off1489;
    std::uint8_t off1490;
    std::uint8_t off1491;
    std::uint8_t off1492;
    std::uint8_t off1493;
    std::uint8_t off1494;
    std::uint8_t off1495;
    std::uint8_t off1496;
    std::uint8_t off1497;
    std::uint8_t off1498;
    std::uint8_t off1499;
    std::uint8_t off1500;
    std::uint8_t off1501;
    std::uint8_t off1502;
    std::uint8_t off1503;
    std::uint8_t off1504;
    std::uint8_t off1505;
    std::uint8_t off1506;
    std::uint8_t off1507;
    std::uint8_t off1508;
    std::uint8_t off1509;
    std::uint8_t off1510;
    std::uint8_t off1511;
    std::uint8_t off1512;
    std::uint8_t off1513;
    std::uint8_t off1514;
    std::uint8_t off1515;
    std::uint8_t off1516;
    std::uint8_t off1517;
    std::uint8_t off1518;
    std::uint8_t off1519;
    std::uint8_t off1520;
    std::uint8_t off1521;
    std::uint8_t off1522;
    std::uint8_t off1523;
    std::uint8_t off1524;
    std::uint8_t off1525;
    std::uint8_t off1526;
    std::uint8_t off1527;
    std::uint8_t off1528;
    std::uint8_t off1529;
    std::uint8_t off1530;
    std::uint8_t off1531;
    std::uint8_t off1532;
    std::uint8_t off1533;
    std::uint8_t off1534;
    std::uint8_t off1535;
    std::uint8_t off1536;
    std::uint8_t off1537;
    std::uint8_t off1538;
    std::uint8_t off1539;
    std::uint8_t off1540;
    std::uint8_t off1541;
    std::uint8_t off1542;
    std::uint8_t off1543;
    std::uint8_t off1544;
    std::uint8_t off1545;
    std::uint8_t off1546;
    std::uint8_t off1547;
    std::uint8_t off1548;
    std::uint8_t off1549;
    std::uint8_t off1550;
    std::uint8_t off1551;
    std::uint8_t off1552;
    std::uint8_t off1553;
    std::uint8_t off1554;
    std::uint8_t off1555;
    std::uint8_t off1556;
    std::uint8_t off1557;
    std::uint8_t off1558;
    std::uint8_t off1559;
    std::uint8_t off1560;
    std::uint8_t off1561;
    std::uint8_t off1562;
    std::uint8_t off1563;
    std::uint8_t off1564;
    std::uint8_t off1565;
    std::uint8_t off1566;
    std::uint8_t off1567;
    std::uint8_t off1568;
    std::uint8_t off1569;
    std::uint8_t off1570;
    std::uint8_t off1571;
    std::uint8_t off1572;
    std::uint8_t off1573;
    std::uint8_t off1574;
    std::uint8_t off1575;
    std::uint8_t off1576;
    std::uint8_t off1577;
    std::uint8_t off1578;
    std::uint8_t off1579;
    std::uint8_t off1580;
    std::uint8_t off1581;
    std::uint8_t off1582;
    std::uint8_t off1583;
    std::uint8_t off1584;
    std::uint8_t off1585;
    std::uint8_t off1586;
    std::uint8_t off1587;
    std::uint8_t off1588;
    std::uint8_t off1589;
    std::uint8_t off1590;
    std::uint8_t off1591;
    std::uint8_t off1592;
    std::uint8_t off1593;
    std::uint8_t off1594;
    std::uint8_t off1595;
    std::uint8_t off1596;
    std::uint8_t off1597;
    std::uint8_t off1598;
    std::uint8_t off1599;
    std::uint8_t off1600;
    std::uint8_t off1601;
    std::uint8_t off1602;
    std::uint8_t off1603;
    std::uint8_t off1604;
    std::uint8_t off1605;
    std::uint8_t off1606;
    std::uint8_t off1607;
    std::uint8_t off1608;
    std::uint8_t off1609;
    std::uint8_t off1610;
    std::uint8_t off1611;
    std::uint8_t off1612;
    std::uint8_t off1613;
    std::uint8_t off1614;
    std::uint8_t off1615;
    std::uint8_t off1616;
    std::uint8_t off1617;
    std::uint8_t off1618;
    std::uint8_t off1619;
    std::uint8_t off1620;
    std::uint8_t off1621;
    std::uint8_t off1622;
    std::uint8_t off1623;
    std::uint8_t off1624;
    std::uint8_t off1625;
    std::uint8_t off1626;
    std::uint8_t off1627;
    std::uint8_t off1628;
    std::uint8_t off1629;
    std::uint8_t off1630;
    std::uint8_t off1631;
    std::uint8_t off1632;
    std::uint8_t off1633;
    std::uint8_t off1634;
    std::uint8_t off1635;
    std::uint8_t off1636;
    std::uint8_t off1637;
    std::uint8_t off1638;
    std::uint8_t off1639;
    std::uint8_t off1640;
    std::uint8_t off1641;
    std::uint8_t off1642;
    std::uint8_t off1643;
    std::uint8_t off1644;
    std::uint8_t off1645;
    std::uint8_t off1646;
    std::uint8_t off1647;
    std::uint8_t off1648;
    std::uint8_t off1649;
    std::uint8_t off1650;
    std::uint8_t off1651;
    std::uint8_t off1652;
    std::uint8_t off1653;
    std::uint8_t off1654;
    std::uint8_t off1655;
    std::uint8_t off1656;
    std::uint8_t off1657;
    std::uint8_t off1658;
    std::uint8_t off1659;
    std::uint8_t off1660;
    std::uint8_t off1661;
    std::uint8_t off1662;
    std::uint8_t off1663;
    std::uint8_t off1664;
    std::uint8_t off1665;
    std::uint8_t off1666;
    std::uint8_t off1667;
    std::uint8_t off1668;
    std::uint8_t off1669;
    std::uint8_t off1670;
    std::uint8_t off1671;
    std::uint8_t off1672;
    std::uint8_t off1673;
    std::uint8_t off1674;
    std::uint8_t off1675;
    std::uint8_t off1676;
    std::uint8_t off1677;
    std::uint8_t off1678;
    std::uint8_t off1679;
    std::uint8_t off1680;
    std::uint8_t off1681;
    std::uint8_t off1682;
    std::uint8_t off1683;
    std::uint8_t off1684;
    std::uint8_t off1685;
    std::uint8_t off1686;
    std::uint8_t off1687;
    std::uint8_t off1688;
    std::uint8_t off1689;
    std::uint8_t off1690;
    std::uint8_t off1691;
    std::uint8_t off1692;
    std::uint8_t off1693;
    std::uint8_t off1694;
    std::uint8_t off1695;
    std::uint8_t off1696;
    std::uint8_t off1697;
    std::uint8_t off1698;
    std::uint8_t off1699;
    std::uint8_t off1700;
    std::uint8_t off1701;
    std::uint8_t off1702;
    std::uint8_t off1703;
    std::uint8_t off1704;
    std::uint8_t off1705;
    std::uint8_t off1706;
    std::uint8_t off1707;
    std::uint8_t off1708;
    std::uint8_t off1709;
    std::uint8_t off1710;
    std::uint8_t off1711;
    std::uint8_t off1712;
    std::uint8_t off1713;
    std::uint8_t off1714;
    std::uint8_t off1715;
    std::uint8_t off1716;
    std::uint8_t off1717;
    std::uint8_t off1718;
    std::uint8_t off1719;
    std::uint8_t off1720;
    std::uint8_t off1721;
    std::uint8_t off1722;
    std::uint8_t off1723;
    std::uint8_t off1724;
    std::uint8_t off1725;
    std::uint8_t off1726;
    std::uint8_t off1727;
    std::uint8_t off1728;
    std::uint8_t off1729;
    std::uint8_t off1730;
    std::uint8_t off1731;
    std::uint8_t off1732;
    std::uint8_t off1733;
    std::uint8_t off1734;
    std::uint8_t off1735;
    std::uint8_t off1736;
    std::uint8_t off1737;
    std::uint8_t off1738;
    std::uint8_t off1739;
    std::uint8_t off1740;
    std::uint8_t off1741;
    std::uint8_t off1742;
    std::uint8_t off1743;
    std::uint8_t off1744;
    std::uint8_t off1745;
    std::uint8_t off1746;
    std::uint8_t off1747;
    std::uint8_t off1748;
    std::uint8_t off1749;
    std::uint8_t off1750;
    std::uint8_t off1751;
    std::uint8_t off1752;
    std::uint8_t off1753;
    std::uint8_t off1754;
    std::uint8_t off1755;
    std::uint8_t off1756;
    std::uint8_t off1757;
    std::uint8_t off1758;
    std::uint8_t off1759;
    std::uint8_t off1760;
    std::uint8_t off1761;
    std::uint8_t off1762;
    std::uint8_t off1763;
    std::uint8_t off1764;
    std::uint8_t off1765;
    std::uint8_t off1766;
    std::uint8_t off1767;
    std::uint8_t off1768;
    std::uint8_t off1769;
    std::uint8_t off1770;
    std::uint8_t off1771;
    std::uint8_t off1772;
    std::uint8_t off1773;
    std::uint8_t off1774;
    std::uint8_t off1775;
    std::uint8_t off1776;
    std::uint8_t off1777;
    std::uint8_t off1778;
    std::uint8_t off1779;
    std::uint8_t off1780;
    std::uint8_t off1781;
    std::uint8_t off1782;
    std::uint8_t off1783;
    std::uint8_t off1784;
    std::uint8_t off1785;
    std::uint8_t off1786;
    std::uint8_t off1787;
    std::uint8_t off1788;
    std::uint8_t off1789;
    std::uint8_t off1790;
    std::uint8_t off1791;
    std::uint8_t off1792;
    std::uint8_t off1793;
    std::uint8_t off1794;
    std::uint8_t off1795;
    std::uint8_t off1796;
    std::uint8_t off1797;
    std::uint8_t off1798;
    std::uint8_t off1799;
    std::uint8_t off1800;
    std::uint8_t off1801;
    std::uint8_t off1802;
    std::uint8_t off1803;
    std::uint8_t off1804;
    std::uint8_t off1805;
    std::uint8_t off1806;
    std::uint8_t off1807;
    std::uint8_t off1808;
    std::uint8_t off1809;
    std::uint8_t off1810;
    std::uint8_t off1811;
    std::uint8_t off1812;
    std::uint8_t off1813;
    std::uint8_t off1814;
    std::uint8_t off1815;
    std::uint8_t off1816;
    std::uint8_t off1817;
    std::uint8_t off1818;
    std::uint8_t off1819;
    std::uint8_t off1820;
    std::uint8_t off1821;
    std::uint8_t off1822;
    std::uint8_t off1823;
    std::uint8_t off1824;
    std::uint8_t off1825;
    std::uint8_t off1826;
    std::uint8_t off1827;
    std::uint8_t off1828;
    std::uint8_t off1829;
    std::uint8_t off1830;
    std::uint8_t off1831;
    std::uint8_t off1832;
    std::uint8_t off1833;
    std::uint8_t off1834;
    std::uint8_t off1835;
    std::uint8_t off1836;
    std::uint8_t off1837;
    std::uint8_t off1838;
    std::uint8_t off1839;
    std::uint8_t off1840;
    std::uint8_t off1841;
    std::uint8_t off1842;
    std::uint8_t off1843;
    std::uint8_t off1844;
    std::uint8_t off1845;
    std::uint8_t off1846;
    std::uint8_t off1847;
    std::uint8_t off1848;
    std::uint8_t off1849;
    std::uint8_t off1850;
    std::uint8_t off1851;
    std::uint8_t off1852;
    std::uint8_t off1853;
    std::uint8_t off1854;
    std::uint8_t off1855;
    std::uint8_t off1856;
    std::uint8_t off1857;
    std::uint8_t off1858;
    std::uint8_t off1859;
    std::uint8_t off1860;
    std::uint8_t off1861;
    std::uint8_t off1862;
    std::uint8_t off1863;
    std::uint8_t off1864;
    std::uint8_t off1865;
    std::uint8_t off1866;
    std::uint8_t off1867;
    std::uint8_t off1868;
    std::uint8_t off1869;
    std::uint8_t off1870;
    std::uint8_t off1871;
    std::uint8_t off1872;
    std::uint8_t off1873;
    std::uint8_t off1874;
    std::uint8_t off1875;
    std::uint8_t off1876;
    std::uint8_t off1877;
    std::uint8_t off1878;
    std::uint8_t off1879;
    std::uint8_t off1880;
    std::uint8_t off1881;
    std::uint8_t off1882;
    std::uint8_t off1883;
    std::uint8_t off1884;
    std::uint8_t off1885;
    std::uint8_t off1886;
    std::uint8_t off1887;
    std::uint8_t off1888;
    std::uint8_t off1889;
    std::uint8_t off1890;
    std::uint8_t off1891;
    std::uint8_t off1892;
    std::uint8_t off1893;
    std::uint8_t off1894;
    std::uint8_t off1895;
    std::uint8_t off1896;
    std::uint8_t off1897;
    std::uint8_t off1898;
    std::uint8_t off1899;
    std::uint8_t off1900;
    std::uint8_t off1901;
    std::uint8_t off1902;
    std::uint8_t off1903;
    std::uint8_t off1904;
    std::uint8_t off1905;
    std::uint8_t off1906;
    std::uint8_t off1907;
    std::uint8_t off1908;
    std::uint8_t off1909;
    std::uint8_t off1910;
    std::uint8_t off1911;
    std::uint8_t off1912;
    std::uint8_t off1913;
    std::uint8_t off1914;
    std::uint8_t off1915;
    std::uint8_t off1916;
    std::uint8_t off1917;
    std::uint8_t off1918;
    std::uint8_t off1919;
    std::uint8_t off1920;
    std::uint8_t off1921;
    std::uint8_t off1922;
    std::uint8_t off1923;
    std::uint8_t off1924;
    std::uint8_t off1925;
    std::uint8_t off1926;
    std::uint8_t off1927;
    std::uint8_t off1928;
    std::uint8_t off1929;
    std::uint8_t off1930;
    std::uint8_t off1931;
    std::uint8_t off1932;
    std::uint8_t off1933;
    std::uint8_t off1934;
    std::uint8_t off1935;
    std::uint8_t off1936;
    std::uint8_t off1937;
    std::uint8_t off1938;
    std::uint8_t off1939;
    std::uint8_t off1940;
    std::uint8_t off1941;
    std::uint8_t off1942;
    std::uint8_t off1943;
    std::uint8_t off1944;
    std::uint8_t off1945;
    std::uint8_t off1946;
    std::uint8_t off1947;
    std::uint8_t off1948;
    std::uint8_t off1949;
    std::uint8_t off1950;
    std::uint8_t off1951;
    std::uint8_t off1952;
    std::uint8_t off1953;
    std::uint8_t off1954;
    std::uint8_t off1955;
    std::uint8_t off1956;
    std::uint8_t off1957;
    std::uint8_t off1958;
    std::uint8_t off1959;
    std::uint8_t off1960;
    std::uint8_t off1961;
    std::uint8_t off1962;
    std::uint8_t off1963;
    std::uint8_t off1964;
    std::uint8_t off1965;
    std::uint8_t off1966;
    std::uint8_t off1967;
    std::uint8_t off1968;
    std::uint8_t off1969;
    std::uint8_t off1970;
    std::uint8_t off1971;
    std::uint8_t off1972;
    std::uint8_t off1973;
    std::uint8_t off1974;
    std::uint8_t off1975;
    std::uint8_t off1976;
    std::uint8_t off1977;
    std::uint8_t off1978;
    std::uint8_t off1979;
    std::uint8_t off1980;
    std::uint8_t off1981;
    std::uint8_t off1982;
    std::uint8_t off1983;
    std::uint8_t off1984;
    std::uint8_t off1985;
    std::uint8_t off1986;
    std::uint8_t off1987;
    std::uint8_t off1988;
    std::uint8_t off1989;
    std::uint8_t off1990;
    std::uint8_t off1991;
    std::uint8_t off1992;
    std::uint8_t off1993;
    std::uint8_t off1994;
    std::uint8_t off1995;
    std::uint8_t off1996;
    std::uint8_t off1997;
    std::uint8_t off1998;
    std::uint8_t off1999;
    std::uint8_t off2000;
    std::uint8_t off2001;
    std::uint8_t off2002;
    std::uint8_t off2003;
    std::uint8_t off2004;
    std::uint8_t off2005;
    std::uint8_t off2006;
    std::uint8_t off2007;
    std::uint8_t off2008;
    std::uint8_t off2009;
    std::uint8_t off2010;
    std::uint8_t off2011;
    std::uint8_t off2012;
    std::uint8_t off2013;
    std::uint8_t off2014;
    std::uint8_t off2015;
    std::uint8_t off2016;
    std::uint8_t off2017;
    std::uint8_t off2018;
    std::uint8_t off2019;
    std::uint8_t off2020;
    std::uint8_t off2021;
    std::uint8_t off2022;
    std::uint8_t off2023;
    std::uint8_t off2024;
    std::uint8_t off2025;
    std::uint8_t off2026;
    std::uint8_t off2027;
    std::uint8_t off2028;
    std::uint8_t off2029;
    std::uint8_t off2030;
    std::uint8_t off2031;
    std::uint8_t off2032;
    std::uint8_t off2033;
    std::uint8_t off2034;
    std::uint8_t off2035;
    std::uint8_t off2036;
    std::uint8_t off2037;
    std::uint8_t off2038;
    std::uint8_t off2039;
    std::uint8_t off2040;
    std::uint8_t off2041;
    std::uint8_t off2042;
    std::uint8_t off2043;
    std::uint8_t off2044;
    std::uint8_t off2045;
    std::uint8_t off2046;
    std::uint8_t off2047;
    std::uint8_t off2048;
    std::uint8_t off2049;
    std::uint8_t off2050;
    std::uint8_t off2051;
    std::uint8_t off2052;
    std::uint8_t off2053;
    std::uint8_t off2054;
    std::uint8_t off2055;
    std::uint8_t off2056;
    std::uint8_t off2057;
    std::uint8_t off2058;
    std::uint8_t off2059;
    std::uint8_t off2060;
    std::uint8_t off2061;
    std::uint8_t off2062;
    std::uint8_t off2063;
    std::uint8_t off2064;
    std::uint8_t off2065;
    std::uint8_t off2066;
    std::uint8_t off2067;
    std::uint8_t off2068;
    std::uint8_t off2069;
    std::uint8_t off2070;
    std::uint8_t off2071;
    std::uint8_t off2072;
    std::uint8_t off2073;
    std::uint8_t off2074;
    std::uint8_t off2075;
    std::uint8_t off2076;
    std::uint8_t off2077;
    std::uint8_t off2078;
    std::uint8_t off2079;
    std::uint8_t off2080;
    std::uint8_t off2081;
    std::uint8_t off2082;
    std::uint8_t off2083;
    std::uint8_t off2084;
    std::uint8_t off2085;
    std::uint8_t off2086;
    std::uint8_t off2087;
    std::uint8_t off2088;
    std::uint8_t off2089;
    std::uint8_t off2090;
    std::uint8_t off2091;
    std::uint8_t off2092;
    std::uint8_t off2093;
    std::uint8_t off2094;
    std::uint8_t off2095;
    std::uint8_t off2096;
    std::uint8_t off2097;
    std::uint8_t off2098;
    std::uint8_t off2099;
    std::uint8_t off2100;
    std::uint8_t off2101;
    std::uint8_t off2102;
    std::uint8_t off2103;
    std::uint8_t off2104;
    std::uint8_t off2105;
    std::uint8_t off2106;
    std::uint8_t off2107;
    std::uint8_t off2108;
    std::uint8_t off2109;
    std::uint8_t off2110;
    std::uint8_t off2111;
    std::uint8_t off2112;
    std::uint8_t off2113;
    std::uint8_t off2114;
    std::uint8_t off2115;
    std::uint8_t off2116;
    std::uint8_t off2117;
    std::uint8_t off2118;
    std::uint8_t off2119;
    std::uint8_t off2120;
    std::uint8_t off2121;
    std::uint8_t off2122;
    std::uint8_t off2123;
    std::uint8_t off2124;
    std::uint8_t off2125;
    std::uint8_t off2126;
    std::uint8_t off2127;
    std::uint8_t off2128;
    std::uint8_t off2129;
    std::uint8_t off2130;
    std::uint8_t off2131;
    std::uint8_t off2132;
    std::uint8_t off2133;
    std::uint8_t off2134;
    std::uint8_t off2135;
    std::uint8_t off2136;
    std::uint8_t off2137;
    std::uint8_t off2138;
    std::uint8_t off2139;
    std::uint8_t off2140;
    std::uint8_t off2141;
    std::uint8_t off2142;
    std::uint8_t off2143;
    std::uint8_t off2144;
    std::uint8_t off2145;
    std::uint8_t off2146;
    std::uint8_t off2147;
    std::uint8_t off2148;
    std::uint8_t off2149;
    std::uint8_t off2150;
    std::uint8_t off2151;
    std::uint8_t off2152;
    std::uint8_t off2153;
    std::uint8_t off2154;
    std::uint8_t off2155;
    std::uint8_t off2156;
    std::uint8_t off2157;
    std::uint8_t off2158;
    std::uint8_t off2159;
    std::uint8_t off2160;
    std::uint8_t off2161;
    std::uint8_t off2162;
    std::uint8_t off2163;
    std::uint8_t off2164;
    std::uint8_t off2165;
    std::uint8_t off2166;
    std::uint8_t off2167;
    std::uint8_t off2168;
    std::uint8_t off2169;
    std::uint8_t off2170;
    std::uint8_t off2171;
    std::uint8_t off2172;
    std::uint8_t off2173;
    std::uint8_t off2174;
    std::uint8_t off2175;
    std::uint8_t off2176;
    std::uint8_t off2177;
    std::uint8_t off2178;
    std::uint8_t off2179;
    std::uint8_t off2180;
    std::uint8_t off2181;
    std::uint8_t off2182;
    std::uint8_t off2183;
    std::uint8_t off2184;
    std::uint8_t off2185;
    std::uint8_t off2186;
    std::uint8_t off2187;
    std::uint8_t off2188;
    std::uint8_t off2189;
    std::uint8_t off2190;
    std::uint8_t off2191;
    std::uint8_t off2192;
    std::uint8_t off2193;
    std::uint8_t off2194;
    std::uint8_t off2195;
    std::uint8_t off2196;
    std::uint8_t off2197;
    std::uint8_t off2198;
    std::uint8_t off2199;
    std::uint8_t off2200;
    std::uint8_t off2201;
    std::uint8_t off2202;
    std::uint8_t off2203;
    std::uint8_t off2204;
    std::uint8_t off2205;
    std::uint8_t off2206;
    std::uint8_t off2207;
    std::uint8_t off2208;
    std::uint8_t off2209;
    std::uint8_t off2210;
    std::uint8_t off2211;
    std::uint8_t off2212;
    std::uint8_t off2213;
    std::uint8_t off2214;
    std::uint8_t off2215;
    std::uint8_t off2216;
    std::uint8_t off2217;
    std::uint8_t off2218;
    std::uint8_t off2219;
    std::uint8_t off2220;
    std::uint8_t off2221;
    std::uint8_t off2222;
    std::uint8_t off2223;
    std::uint8_t off2224;
    std::uint8_t off2225;
    std::uint8_t off2226;
    std::uint8_t off2227;
    std::uint8_t off2228;
    std::uint8_t off2229;
    std::uint8_t off2230;
    std::uint8_t off2231;
    std::uint8_t off2232;
    std::uint8_t off2233;
    std::uint8_t off2234;
    std::uint8_t off2235;
    std::uint8_t off2236;
    std::uint8_t off2237;
    std::uint8_t off2238;
    std::uint8_t off2239;
    std::uint8_t off2240;
    std::uint8_t off2241;
    std::uint8_t off2242;
    std::uint8_t off2243;
    std::uint8_t off2244;
    std::uint8_t off2245;
    std::uint8_t off2246;
    std::uint8_t off2247;
    std::uint8_t off2248;
    std::uint8_t off2249;
    std::uint8_t off2250;
    std::uint8_t off2251;
    std::uint8_t off2252;
    std::uint8_t off2253;
    std::uint8_t off2254;
    std::uint8_t off2255;
    std::uint8_t off2256;
    std::uint8_t off2257;
    std::uint8_t off2258;
    std::uint8_t off2259;
    std::uint8_t off2260;
    std::uint8_t off2261;
    std::uint8_t off2262;
    std::uint8_t off2263;
    std::uint8_t off2264;
    std::uint8_t off2265;
    std::uint8_t off2266;
    std::uint8_t off2267;
    std::uint8_t off2268;
    std::uint8_t off2269;
    std::uint8_t off2270;
    std::uint8_t off2271;
    std::uint8_t off2272;
    std::uint8_t off2273;
    std::uint8_t off2274;
    std::uint8_t off2275;
    std::uint8_t off2276;
    std::uint8_t off2277;
    std::uint8_t off2278;
    std::uint8_t off2279;
    std::uint8_t off2280;
    std::uint8_t off2281;
    std::uint8_t off2282;
    std::uint8_t off2283;
    std::uint8_t off2284;
    std::uint8_t off2285;
    std::uint8_t off2286;
    std::uint8_t off2287;
    std::uint8_t off2288;
    std::uint8_t off2289;
    std::uint8_t off2290;
    std::uint8_t off2291;
    std::uint8_t off2292;
    std::uint8_t off2293;
    std::uint8_t off2294;
    std::uint8_t off2295;
    std::uint8_t off2296;
    std::uint8_t off2297;
    std::uint8_t off2298;
    std::uint8_t off2299;
    std::uint8_t off2300;
    std::uint8_t off2301;
    std::uint8_t off2302;
    std::uint8_t off2303;
    std::uint8_t off2304;
    std::uint8_t off2305;
    std::uint8_t off2306;
    std::uint8_t off2307;
    std::uint8_t off2308;
    std::uint8_t off2309;
    std::uint8_t off2310;
    std::uint8_t off2311;
    std::uint8_t off2312;
    std::uint8_t off2313;
    std::uint8_t off2314;
    std::uint8_t off2315;
    std::uint8_t off2316;
    std::uint8_t off2317;
    std::uint8_t off2318;
    std::uint8_t off2319;
    std::uint8_t off2320;
    std::uint8_t off2321;
    std::uint8_t off2322;
    std::uint8_t off2323;
    std::uint8_t off2324;
    std::uint8_t off2325;
    std::uint8_t off2326;
    std::uint8_t off2327;
    std::uint8_t off2328;
    std::uint8_t off2329;
    std::uint8_t off2330;
    std::uint8_t off2331;
    std::uint8_t off2332;
    std::uint8_t off2333;
    std::uint8_t off2334;
    std::uint8_t off2335;
    std::uint8_t off2336;
    std::uint8_t off2337;
    std::uint8_t off2338;
    std::uint8_t off2339;
    std::uint8_t off2340;
    std::uint8_t off2341;
    std::uint8_t off2342;
    std::uint8_t off2343;
    std::uint8_t off2344;
    std::uint8_t off2345;
    std::uint8_t off2346;
    std::uint8_t off2347;
    std::uint8_t off2348;
    std::uint8_t off2349;
    std::uint8_t off2350;
    std::uint8_t off2351;
    std::uint8_t off2352;
    std::uint8_t off2353;
    std::uint8_t off2354;
    std::uint8_t off2355;
    std::uint8_t off2356;
    std::uint8_t off2357;
    std::uint8_t off2358;
    std::uint8_t off2359;
    std::uint8_t off2360;
    std::uint8_t off2361;
    std::uint8_t off2362;
    std::uint8_t off2363;
    std::uint8_t off2364;
    std::uint8_t off2365;
    std::uint8_t off2366;
    std::uint8_t off2367;
    std::uint8_t off2368;
    std::uint8_t off2369;
    std::uint8_t off2370;
    std::uint8_t off2371;
    std::uint8_t off2372;
    std::uint8_t off2373;
    std::uint8_t off2374;
    std::uint8_t off2375;
    std::uint8_t off2376;
    std::uint8_t off2377;
    std::uint8_t off2378;
    std::uint8_t off2379;
    std::uint8_t off2380;
    std::uint8_t off2381;
    std::uint8_t off2382;
    std::uint8_t off2383;
    std::uint8_t off2384;
    std::uint8_t off2385;
    std::uint8_t off2386;
    std::uint8_t off2387;
    std::uint8_t off2388;
    std::uint8_t off2389;
    std::uint8_t off2390;
    std::uint8_t off2391;
    std::uint8_t off2392;
    std::uint8_t off2393;
    std::uint8_t off2394;
    std::uint8_t off2395;
    std::uint8_t off2396;
    std::uint8_t off2397;
    std::uint8_t off2398;
    std::uint8_t off2399;
    std::uint8_t off2400;
    std::uint8_t off2401;
    std::uint8_t off2402;
    std::uint8_t off2403;
    std::uint8_t off2404;
    std::uint8_t off2405;
    std::uint8_t off2406;
    std::uint8_t off2407;
    std::uint8_t off2408;
    std::uint8_t off2409;
    std::uint8_t off2410;
    std::uint8_t off2411;
    std::uint8_t off2412;
    std::uint8_t off2413;
    std::uint8_t off2414;
    std::uint8_t off2415;
    std::uint8_t off2416;
    std::uint8_t off2417;
    std::uint8_t off2418;
    std::uint8_t off2419;
    std::uint8_t off2420;
    std::uint8_t off2421;
    std::uint8_t off2422;
    std::uint8_t off2423;
    std::uint8_t off2424;
    std::uint8_t off2425;
    std::uint8_t off2426;
    std::uint8_t off2427;
    std::uint8_t off2428;
    std::uint8_t off2429;
    std::uint8_t off2430;
    std::uint8_t off2431;
    std::uint8_t off2432;
    std::uint8_t off2433;
    std::uint8_t off2434;
    std::uint8_t off2435;
    std::uint8_t off2436;
    std::uint8_t off2437;
    std::uint8_t off2438;
    std::uint8_t off2439;
    std::uint8_t off2440;
    std::uint8_t off2441;
    std::uint8_t off2442;
    std::uint8_t off2443;
    std::uint8_t off2444;
    std::uint8_t off2445;
    std::uint8_t off2446;
    std::uint8_t off2447;
    std::uint8_t off2448;
    std::uint8_t off2449;
    std::uint8_t off2450;
    std::uint8_t off2451;
    std::uint8_t off2452;
    std::uint8_t off2453;
    std::uint8_t off2454;
    std::uint8_t off2455;
    std::uint8_t off2456;
    std::uint8_t off2457;
    std::uint8_t off2458;
    std::uint8_t off2459;
    std::uint8_t off2460;
    std::uint8_t off2461;
    std::uint8_t off2462;
    std::uint8_t off2463;
    std::uint8_t off2464;
    std::uint8_t off2465;
    std::uint8_t off2466;
    std::uint8_t off2467;
    std::uint8_t off2468;
    std::uint8_t off2469;
    std::uint8_t off2470;
    std::uint8_t off2471;
    std::uint8_t off2472;
    std::uint8_t off2473;
    std::uint8_t off2474;
    std::uint8_t off2475;
    std::uint8_t off2476;
    std::uint8_t off2477;
    std::uint8_t off2478;
    std::uint8_t off2479;
    std::uint8_t off2480;
    std::uint8_t off2481;
    std::uint8_t off2482;
    std::uint8_t off2483;
    std::uint8_t off2484;
    std::uint8_t off2485;
    std::uint8_t off2486;
    std::uint8_t off2487;
    std::uint8_t off2488;
    std::uint8_t off2489;
    std::uint8_t off2490;
    std::uint8_t off2491;
    std::uint8_t off2492;
    std::uint8_t off2493;
    std::uint8_t off2494;
    std::uint8_t off2495;
    std::uint8_t off2496;
    std::uint8_t off2497;
    std::uint8_t off2498;
    std::uint8_t off2499;
    std::uint8_t off2500;
    std::uint8_t off2501;
    std::uint8_t off2502;
    std::uint8_t off2503;
    std::uint8_t off2504;
    std::uint8_t off2505;
    std::uint8_t off2506;
    std::uint8_t off2507;
    std::uint8_t off2508;
    std::uint8_t off2509;
    std::uint8_t off2510;
    std::uint8_t off2511;
    std::uint8_t off2512;
    std::uint8_t off2513;
    std::uint8_t off2514;
    std::uint8_t off2515;
    std::uint8_t off2516;
    std::uint8_t off2517;
    std::uint8_t off2518;
    std::uint8_t off2519;
    std::uint8_t off2520;
    std::uint8_t off2521;
    std::uint8_t off2522;
    std::uint8_t off2523;
    std::uint8_t off2524;
    std::uint8_t off2525;
    std::uint8_t off2526;
    std::uint8_t off2527;
    std::uint8_t off2528;
    std::uint8_t off2529;
    std::uint8_t off2530;
    std::uint8_t off2531;
    std::uint8_t off2532;
    std::uint8_t off2533;
    std::uint8_t off2534;
    std::uint8_t off2535;
    std::uint8_t off2536;
    std::uint8_t off2537;
    std::uint8_t off2538;
    std::uint8_t off2539;
    std::uint8_t off2540;
    std::uint8_t off2541;
    std::uint8_t off2542;
    std::uint8_t off2543;
    std::uint8_t off2544;
    std::uint8_t off2545;
    std::uint8_t off2546;
    std::uint8_t off2547;
    std::uint8_t off2548;
    std::uint8_t off2549;
    std::uint8_t off2550;
    std::uint8_t off2551;
    std::uint8_t off2552;
    std::uint8_t off2553;
    std::uint8_t off2554;
    std::uint8_t off2555;
    std::uint8_t off2556;
    std::uint8_t off2557;
    std::uint8_t off2558;
    std::uint8_t off2559;
    std::uint8_t off2560;
    std::uint8_t off2561;
    std::uint8_t off2562;
    std::uint8_t off2563;
    std::uint8_t off2564;
    std::uint8_t off2565;
    std::uint8_t off2566;
    std::uint8_t off2567;
    std::uint8_t off2568;
    std::uint8_t off2569;
    std::uint8_t off2570;
    std::uint8_t off2571;
    std::uint8_t off2572;
    std::uint8_t off2573;
    std::uint8_t off2574;
    std::uint8_t off2575;
    std::uint8_t off2576;
    std::uint8_t off2577;
    std::uint8_t off2578;
    std::uint8_t off2579;
    std::uint8_t off2580;
    std::uint8_t off2581;
    std::uint8_t off2582;
    std::uint8_t off2583;
    std::uint8_t off2584;
    std::uint8_t off2585;
    std::uint8_t off2586;
    std::uint8_t off2587;
    std::uint8_t off2588;
    std::uint8_t off2589;
    std::uint8_t off2590;
    std::uint8_t off2591;
    std::uint8_t off2592;
    std::uint8_t off2593;
    std::uint8_t off2594;
    std::uint8_t off2595;
    std::uint8_t off2596;
    std::uint8_t off2597;
    std::uint8_t off2598;
    std::uint8_t off2599;
    std::uint8_t off2600;
    std::uint8_t off2601;
    std::uint8_t off2602;
    std::uint8_t off2603;
    std::uint8_t off2604;
    std::uint8_t off2605;
    std::uint8_t off2606;
    std::uint8_t off2607;
    std::uint8_t off2608;
    std::uint8_t off2609;
    std::uint8_t off2610;
    std::uint8_t off2611;
    std::uint8_t off2612;
    std::uint8_t off2613;
    std::uint8_t off2614;
    std::uint8_t off2615;
    std::uint8_t off2616;
    std::uint8_t off2617;
    std::uint8_t off2618;
    std::uint8_t off2619;
    std::uint8_t off2620;
    std::uint8_t off2621;
    std::uint8_t off2622;
    std::uint8_t off2623;
    std::uint8_t off2624;
    std::uint8_t off2625;
    std::uint8_t off2626;
    std::uint8_t off2627;
    std::uint8_t off2628;
    std::uint8_t off2629;
    std::uint8_t off2630;
    std::uint8_t off2631;
    std::uint8_t off2632;
    std::uint8_t off2633;
    std::uint8_t off2634;
    std::uint8_t off2635;
    std::uint8_t off2636;
    std::uint8_t off2637;
    std::uint8_t off2638;
    std::uint8_t off2639;
    std::uint8_t off2640;
    std::uint8_t off2641;
    std::uint8_t off2642;
    std::uint8_t off2643;
    std::uint8_t off2644;
    std::uint8_t off2645;
    std::uint8_t off2646;
    std::uint8_t off2647;
    std::uint8_t off2648;
    std::uint8_t off2649;
    std::uint8_t off2650;
    std::uint8_t off2651;
    std::uint8_t off2652;
    std::uint8_t off2653;
    std::uint8_t off2654;
    std::uint8_t off2655;
    std::uint8_t off2656;
    std::uint8_t off2657;
    std::uint8_t off2658;
    std::uint8_t off2659;
    std::uint8_t off2660;
    std::uint8_t off2661;
    std::uint8_t off2662;
    std::uint8_t off2663;
    std::uint8_t off2664;
    std::uint8_t off2665;
    std::uint8_t off2666;
    std::uint8_t off2667;
    std::uint8_t off2668;
    std::uint8_t off2669;
    std::uint8_t off2670;
    std::uint8_t off2671;
    std::uint8_t off2672;
    std::uint8_t off2673;
    std::uint8_t off2674;
    std::uint8_t off2675;
    std::uint8_t off2676;
    std::uint8_t off2677;
    std::uint8_t off2678;
    std::uint8_t off2679;
    std::uint8_t off2680;
    std::uint8_t off2681;
    std::uint8_t off2682;
    std::uint8_t off2683;
    std::uint8_t off2684;
    std::uint8_t off2685;
    std::uint8_t off2686;
    std::uint8_t off2687;
    std::uint8_t off2688;
    std::uint8_t off2689;
    std::uint8_t off2690;
    std::uint8_t off2691;
    std::uint8_t off2692;
    std::uint8_t off2693;
    std::uint8_t off2694;
    std::uint8_t off2695;
    std::uint8_t off2696;
    std::uint8_t off2697;
    std::uint8_t off2698;
    std::uint8_t off2699;
    std::uint8_t off2700;
    std::uint8_t off2701;
    std::uint8_t off2702;
    std::uint8_t off2703;
    std::uint8_t off2704;
    std::uint8_t off2705;
    std::uint8_t off2706;
    std::uint8_t off2707;
    std::uint8_t off2708;
    std::uint8_t off2709;
    std::uint8_t off2710;
    std::uint8_t off2711;
    std::uint8_t off2712;
    std::uint8_t off2713;
    std::uint8_t off2714;
    std::uint8_t off2715;
    std::uint8_t off2716;
    std::uint8_t off2717;
    std::uint8_t off2718;
    std::uint8_t off2719;
    std::uint8_t off2720;
    std::uint8_t off2721;
    std::uint8_t off2722;
    std::uint8_t off2723;
    std::uint8_t off2724;
    std::uint8_t off2725;
    std::uint8_t off2726;
    std::uint8_t off2727;
    std::uint8_t off2728;
    std::uint8_t off2729;
    std::uint8_t off2730;
    std::uint8_t off2731;
    std::uint8_t off2732;
    std::uint8_t off2733;
    std::uint8_t off2734;
    std::uint8_t off2735;
    std::uint8_t off2736;
    std::uint8_t off2737;
    std::uint8_t off2738;
    std::uint8_t off2739;
    std::uint8_t off2740;
    std::uint8_t off2741;
    std::uint8_t off2742;
    std::uint8_t off2743;
    std::uint8_t off2744;
    std::uint8_t off2745;
    std::uint8_t off2746;
    std::uint8_t off2747;
    std::uint8_t off2748;
    std::uint8_t off2749;
    std::uint8_t off2750;
    std::uint8_t off2751;
    std::uint8_t off2752;
    std::uint8_t off2753;
    std::uint8_t off2754;
    std::uint8_t off2755;
    std::uint8_t off2756;
    std::uint8_t off2757;
    std::uint8_t off2758;
    std::uint8_t off2759;
    std::uint8_t off2760;
    std::uint8_t off2761;
    std::uint8_t off2762;
    std::uint8_t off2763;
    std::uint8_t off2764;
    std::uint8_t off2765;
    std::uint8_t off2766;
    std::uint8_t off2767;
    std::uint8_t off2768;
    std::uint8_t off2769;
    std::uint8_t off2770;
    std::uint8_t off2771;
    std::uint8_t off2772;
    std::uint8_t off2773;
    std::uint8_t off2774;
    std::uint8_t off2775;
    std::uint8_t off2776;
    std::uint8_t off2777;
    std::uint8_t off2778;
    std::uint8_t off2779;
    std::uint8_t off2780;
    std::uint8_t off2781;
    std::uint8_t off2782;
    std::uint8_t off2783;
    std::uint8_t off2784;
    std::uint8_t off2785;
    std::uint8_t off2786;
    std::uint8_t off2787;
    std::uint8_t off2788;
    std::uint8_t off2789;
    std::uint8_t off2790;
    std::uint8_t off2791;
    std::uint8_t off2792;
    std::uint8_t off2793;
    std::uint8_t off2794;
    std::uint8_t off2795;
    std::uint8_t off2796;
    std::uint8_t off2797;
    std::uint8_t off2798;
    std::uint8_t off2799;
    std::uint8_t off2800;
    std::uint8_t off2801;
    std::uint8_t off2802;
    std::uint8_t off2803;
    std::uint8_t off2804;
    std::uint8_t off2805;
    std::uint8_t off2806;
    std::uint8_t off2807;
    std::uint8_t off2808;
    std::uint8_t off2809;
    std::uint8_t off2810;
    std::uint8_t off2811;
    std::uint8_t off2812;
    std::uint8_t off2813;
    std::uint8_t off2814;
    std::uint8_t off2815;
    std::uint8_t off2816;
    std::uint8_t off2817;
    std::uint8_t off2818;
    std::uint8_t off2819;
    std::uint8_t off2820;
    std::uint8_t off2821;
    std::uint8_t off2822;
    std::uint8_t off2823;
    std::uint8_t off2824;
    std::uint8_t off2825;
    std::uint8_t off2826;
    std::uint8_t off2827;
    std::uint8_t off2828;
    std::uint8_t off2829;
    std::uint8_t off2830;
    std::uint8_t off2831;
    std::uint8_t off2832;
    std::uint8_t off2833;
    std::uint8_t off2834;
    std::uint8_t off2835;
    std::uint8_t off2836;
    std::uint8_t off2837;
    std::uint8_t off2838;
    std::uint8_t off2839;
    std::uint8_t off2840;
    std::uint8_t off2841;
    std::uint8_t off2842;
    std::uint8_t off2843;
    std::uint8_t off2844;
    std::uint8_t off2845;
    std::uint8_t off2846;
    std::uint8_t off2847;
    std::uint8_t off2848;
    std::uint8_t off2849;
    std::uint8_t off2850;
    std::uint8_t off2851;
    std::uint8_t off2852;
    std::uint8_t off2853;
    std::uint8_t off2854;
    std::uint8_t off2855;
    std::uint8_t off2856;
    std::uint8_t off2857;
    std::uint8_t off2858;
    std::uint8_t off2859;
    std::uint8_t off2860;
    std::uint8_t off2861;
    std::uint8_t off2862;
    std::uint8_t off2863;
    std::uint8_t off2864;
    std::uint8_t off2865;
    std::uint8_t off2866;
    std::uint8_t off2867;
    std::uint8_t off2868;
    std::uint8_t off2869;
    std::uint8_t off2870;
    std::uint8_t off2871;
    std::uint8_t off2872;
    std::uint8_t off2873;
    std::uint8_t off2874;
    std::uint8_t off2875;
    std::uint8_t off2876;
    std::uint8_t off2877;
    std::uint8_t off2878;
    std::uint8_t off2879;
    std::uint8_t off2880;
    std::uint8_t off2881;
    std::uint8_t off2882;
    std::uint8_t off2883;
    std::uint8_t off2884;
    std::uint8_t off2885;
    std::uint8_t off2886;
    std::uint8_t off2887;
    std::uint8_t off2888;
    std::uint8_t off2889;
    std::uint8_t off2890;
    std::uint8_t off2891;
    std::uint8_t off2892;
    std::uint8_t off2893;
    std::uint8_t off2894;
    std::uint8_t off2895;
    std::uint8_t off2896;
    std::uint8_t off2897;
    std::uint8_t off2898;
    std::uint8_t off2899;
    std::uint8_t off2900;
    std::uint8_t off2901;
    std::uint8_t off2902;
    std::uint8_t off2903;
    std::uint8_t off2904;
    std::uint8_t off2905;
    std::uint8_t off2906;
    std::uint8_t off2907;
    std::uint8_t off2908;
    std::uint8_t off2909;
    std::uint8_t off2910;
    std::uint8_t off2911;
    std::uint8_t off2912;
    std::uint8_t off2913;
    std::uint8_t off2914;
    std::uint8_t off2915;
    std::uint8_t off2916;
    std::uint8_t off2917;
    std::uint8_t off2918;
    std::uint8_t off2919;
    std::uint8_t off2920;
    std::uint8_t off2921;
    std::uint8_t off2922;
    std::uint8_t off2923;
    std::uint8_t off2924;
    std::uint8_t off2925;
    std::uint8_t off2926;
    std::uint8_t off2927;
    std::uint8_t off2928;
    std::uint8_t off2929;
    std::uint8_t off2930;
    std::uint8_t off2931;
    std::uint8_t off2932;
    std::uint8_t off2933;
    std::uint8_t off2934;
    std::uint8_t off2935;
    std::uint8_t off2936;
    std::uint8_t off2937;
    std::uint8_t off2938;
    std::uint8_t off2939;
    std::uint8_t off2940;
    std::uint8_t off2941;
    std::uint8_t off2942;
    std::uint8_t off2943;
    std::uint8_t off2944;
    std::uint8_t off2945;
    std::uint8_t off2946;
    std::uint8_t off2947;
    std::uint8_t off2948;
    std::uint8_t off2949;
    std::uint8_t off2950;
    std::uint8_t off2951;
    std::uint8_t off2952;
    std::uint8_t off2953;
    std::uint8_t off2954;
    std::uint8_t off2955;
    std::uint8_t off2956;
    std::uint8_t off2957;
    std::uint8_t off2958;
    std::uint8_t off2959;
    std::uint8_t off2960;
    std::uint8_t off2961;
    std::uint8_t off2962;
    std::uint8_t off2963;
    std::uint8_t off2964;
    std::uint8_t off2965;
    std::uint8_t off2966;
    std::uint8_t off2967;
    std::uint8_t off2968;
    std::uint8_t off2969;
    std::uint8_t off2970;
    std::uint8_t off2971;
    std::uint8_t off2972;
    std::uint8_t off2973;
    std::uint8_t off2974;
    std::uint8_t off2975;
    std::uint8_t off2976;
    std::uint8_t off2977;
    std::uint8_t off2978;
    std::uint8_t off2979;
    std::uint8_t off2980;
    std::uint8_t off2981;
    std::uint8_t off2982;
    std::uint8_t off2983;
    std::uint8_t off2984;
    std::uint8_t off2985;
    std::uint8_t off2986;
    std::uint8_t off2987;
    std::uint8_t off2988;
    std::uint8_t off2989;
    std::uint8_t off2990;
    std::uint8_t off2991;
    std::uint8_t off2992;
    std::uint8_t off2993;
    std::uint8_t off2994;
    std::uint8_t off2995;
    std::uint8_t off2996;
    std::uint8_t off2997;
    std::uint8_t off2998;
    std::uint8_t off2999;
    std::uint8_t off3000;
    std::uint8_t off3001;
    std::uint8_t off3002;
    std::uint8_t off3003;
    std::uint8_t off3004;
    std::uint8_t off3005;
    std::uint8_t off3006;
    std::uint8_t off3007;
    std::uint8_t off3008;
    std::uint8_t off3009;
    std::uint8_t off3010;
    std::uint8_t off3011;
    std::uint8_t off3012;
    std::uint8_t off3013;
    std::uint8_t off3014;
    std::uint8_t off3015;
    std::uint8_t off3016;
    std::uint8_t off3017;
    std::uint8_t off3018;
    std::uint8_t off3019;
    std::uint8_t off3020;
    std::uint8_t off3021;
    std::uint8_t off3022;
    std::uint8_t off3023;
    std::uint8_t off3024;
    std::uint8_t off3025;
    std::uint8_t off3026;
    std::uint8_t off3027;
    std::uint8_t off3028;
    std::uint8_t off3029;
    std::uint8_t off3030;
    std::uint8_t off3031;
    std::uint8_t off3032;
    std::uint8_t off3033;
    std::uint8_t off3034;
    std::uint8_t off3035;
    std::uint8_t off3036;
    std::uint8_t off3037;
    std::uint8_t off3038;
    std::uint8_t off3039;
    std::uint8_t off3040;
    std::uint8_t off3041;
    std::uint8_t off3042;
    std::uint8_t off3043;
    std::uint8_t off3044;
    std::uint8_t off3045;
    std::uint8_t off3046;
    std::uint8_t off3047;
    std::uint8_t off3048;
    std::uint8_t off3049;
    std::uint8_t off3050;
    std::uint8_t off3051;
    std::uint8_t off3052;
    std::uint8_t off3053;
    std::uint8_t off3054;
    std::uint8_t off3055;
    std::uint8_t off3056;
    std::uint8_t off3057;
    std::uint8_t off3058;
    std::uint8_t off3059;
    std::uint8_t off3060;
    std::uint8_t off3061;
    std::uint8_t off3062;
    std::uint8_t off3063;
    std::uint8_t off3064;
    std::uint8_t off3065;
    std::uint8_t off3066;
    std::uint8_t off3067;
    std::uint8_t off3068;
    std::uint8_t off3069;
    std::uint8_t off3070;
    std::uint8_t off3071;
    std::uint8_t off3072;
    std::uint8_t off3073;
    std::uint8_t off3074;
    std::uint8_t off3075;
    std::uint8_t off3076;
    std::uint8_t off3077;
    std::uint8_t off3078;
    std::uint8_t off3079;
    std::uint8_t off3080;
    std::uint8_t off3081;
    std::uint8_t off3082;
    std::uint8_t off3083;
    std::uint8_t off3084;
    std::uint8_t off3085;
    std::uint8_t off3086;
    std::uint8_t off3087;
    std::uint8_t off3088;
    std::uint8_t off3089;
    std::uint8_t off3090;
    std::uint8_t off3091;
    std::uint8_t off3092;
    std::uint8_t off3093;
    std::uint8_t off3094;
    std::uint8_t off3095;
    std::uint8_t off3096;
    std::uint8_t off3097;
    std::uint8_t off3098;
    std::uint8_t off3099;
    std::uint8_t off3100;
    std::uint8_t off3101;
    std::uint8_t off3102;
    std::uint8_t off3103;
    std::uint8_t off3104;
    std::uint8_t off3105;
    std::uint8_t off3106;
    std::uint8_t off3107;
    std::uint8_t off3108;
    std::uint8_t off3109;
    std::uint8_t off3110;
    std::uint8_t off3111;
    std::uint8_t off3112;
    std::uint8_t off3113;
    std::uint8_t off3114;
    std::uint8_t off3115;
    std::uint8_t off3116;
    std::uint8_t off3117;
    std::uint8_t off3118;
    std::uint8_t off3119;
    std::uint8_t off3120;
    std::uint8_t off3121;
    std::uint8_t off3122;
    std::uint8_t off3123;
    std::uint8_t off3124;
    std::uint8_t off3125;
    std::uint8_t off3126;
    std::uint8_t off3127;
    std::uint8_t off3128;
    std::uint8_t off3129;
    std::uint8_t off3130;
    std::uint8_t off3131;
    std::uint8_t off3132;
    std::uint8_t off3133;
    std::uint8_t off3134;
    std::uint8_t off3135;
    std::uint8_t off3136;
    std::uint8_t off3137;
    std::uint8_t off3138;
    std::uint8_t off3139;
    std::uint8_t off3140;
    std::uint8_t off3141;
    std::uint8_t off3142;
    std::uint8_t off3143;
    std::uint8_t off3144;
    std::uint8_t off3145;
    std::uint8_t off3146;
    std::uint8_t off3147;
    std::uint8_t off3148;
    std::uint8_t off3149;
    std::uint8_t off3150;
    std::uint8_t off3151;
    std::uint8_t off3152;
    std::uint8_t off3153;
    std::uint8_t off3154;
    std::uint8_t off3155;
    std::uint8_t off3156;
    std::uint8_t off3157;
    std::uint8_t off3158;
    std::uint8_t off3159;
    std::uint8_t off3160;
    std::uint8_t off3161;
    std::uint8_t off3162;
    std::uint8_t off3163;
    std::uint8_t off3164;
    std::uint8_t off3165;
    std::uint8_t off3166;
    std::uint8_t off3167;
    std::uint8_t off3168;
    std::uint8_t off3169;
    std::uint8_t off3170;
    std::uint8_t off3171;
    std::uint8_t off3172;
    std::uint8_t off3173;
    std::uint8_t off3174;
    std::uint8_t off3175;
    std::uint8_t off3176;
    std::uint8_t off3177;
    std::uint8_t off3178;
    std::uint8_t off3179;
    std::uint8_t off3180;
    std::uint8_t off3181;
    std::uint8_t off3182;
    std::uint8_t off3183;
    std::uint8_t off3184;
    std::uint8_t off3185;
    std::uint8_t off3186;
    std::uint8_t off3187;
    std::uint8_t off3188;
    std::uint8_t off3189;
    std::uint8_t off3190;
    std::uint8_t off3191;
    std::uint8_t off3192;
    std::uint8_t off3193;
    std::uint8_t off3194;
    std::uint8_t off3195;
    std::uint8_t off3196;
    std::uint8_t off3197;
    std::uint8_t off3198;
    std::uint8_t off3199;
    std::uint8_t off3200;
    std::uint8_t off3201;
    std::uint8_t off3202;
    std::uint8_t off3203;
    std::uint8_t off3204;
    std::uint8_t off3205;
    std::uint8_t off3206;
    std::uint8_t off3207;
    std::uint8_t off3208;
    std::uint8_t off3209;
    std::uint8_t off3210;
    std::uint8_t off3211;
    std::uint8_t off3212;
    std::uint8_t off3213;
    std::uint8_t off3214;
    std::uint8_t off3215;
    std::uint8_t off3216;
    std::uint8_t off3217;
    std::uint8_t off3218;
    std::uint8_t off3219;
    std::uint8_t off3220;
    std::uint8_t off3221;
    std::uint8_t off3222;
    std::uint8_t off3223;
    std::uint8_t off3224;
    std::uint8_t off3225;
    std::uint8_t off3226;
    std::uint8_t off3227;
    std::uint8_t off3228;
    std::uint8_t off3229;
    std::uint8_t off3230;
    std::uint8_t off3231;
    std::uint8_t off3232;
    std::uint8_t off3233;
    std::uint8_t off3234;
    std::uint8_t off3235;
    std::uint8_t off3236;
    std::uint8_t off3237;
    std::uint8_t off3238;
    std::uint8_t off3239;
    std::uint8_t off3240;
    std::uint8_t off3241;
    std::uint8_t off3242;
    std::uint8_t off3243;
    std::uint8_t off3244;
    std::uint8_t off3245;
    std::uint8_t off3246;
    std::uint8_t off3247;
    std::uint8_t off3248;
    std::uint8_t off3249;
    std::uint8_t off3250;
    std::uint8_t off3251;
    std::uint8_t off3252;
    std::uint8_t off3253;
    std::uint8_t off3254;
    std::uint8_t off3255;
    std::uint8_t off3256;
    std::uint8_t off3257;
    std::uint8_t off3258;
    std::uint8_t off3259;
    std::uint8_t off3260;
    std::uint8_t off3261;
    std::uint8_t off3262;
    std::uint8_t off3263;
    std::uint8_t off3264;
    std::uint8_t off3265;
    std::uint8_t off3266;
    std::uint8_t off3267;
    std::uint8_t off3268;
    std::uint8_t off3269;
    std::uint8_t off3270;
    std::uint8_t off3271;
    std::uint8_t off3272;
    std::uint8_t off3273;
    std::uint8_t off3274;
    std::uint8_t off3275;
    std::uint8_t off3276;
    std::uint8_t off3277;
    std::uint8_t off3278;
    std::uint8_t off3279;
    std::uint8_t off3280;
    std::uint8_t off3281;
    std::uint8_t off3282;
    std::uint8_t off3283;
    std::uint8_t off3284;
    std::uint8_t off3285;
    std::uint8_t off3286;
    std::uint8_t off3287;
    std::uint8_t off3288;
    std::uint8_t off3289;
    std::uint8_t off3290;
    std::uint8_t off3291;
    std::uint8_t off3292;
    std::uint8_t off3293;
    std::uint8_t off3294;
    std::uint8_t off3295;
    std::uint8_t off3296;
    std::uint8_t off3297;
    std::uint8_t off3298;
    std::uint8_t off3299;
    std::uint8_t off3300;
    std::uint8_t off3301;
    std::uint8_t off3302;
    std::uint8_t off3303;
    std::uint8_t off3304;
    std::uint8_t off3305;
    std::uint8_t off3306;
    std::uint8_t off3307;
    std::uint8_t off3308;
    std::uint8_t off3309;
    std::uint8_t off3310;
    std::uint8_t off3311;
    std::uint8_t off3312;
    std::uint8_t off3313;
    std::uint8_t off3314;
    std::uint8_t off3315;
    std::uint8_t off3316;
    std::uint8_t off3317;
    std::uint8_t off3318;
    std::uint8_t off3319;
    std::uint8_t off3320;
    std::uint8_t off3321;
    std::uint8_t off3322;
    std::uint8_t off3323;
    std::uint8_t off3324;
    std::uint8_t off3325;
    std::uint8_t off3326;
    std::uint8_t off3327;
    std::uint8_t off3328;
    std::uint8_t off3329;
    std::uint8_t off3330;
    std::uint8_t off3331;
    std::uint8_t off3332;
    std::uint8_t off3333;
    std::uint8_t off3334;
    std::uint8_t off3335;
    std::uint8_t off3336;
    std::uint8_t off3337;
    std::uint8_t off3338;
    std::uint8_t off3339;
    std::uint8_t off3340;
    std::uint8_t off3341;
    std::uint8_t off3342;
    std::uint8_t off3343;
    std::uint8_t off3344;
    std::uint8_t off3345;
    std::uint8_t off3346;
    std::uint8_t off3347;
    std::uint8_t off3348;
    std::uint8_t off3349;
    std::uint8_t off3350;
    std::uint8_t off3351;
    std::uint8_t off3352;
    std::uint8_t off3353;
    std::uint8_t off3354;
    std::uint8_t off3355;
    std::uint8_t off3356;
    std::uint8_t off3357;
    std::uint8_t off3358;
    std::uint8_t off3359;
    std::uint8_t off3360;
    std::uint8_t off3361;
    std::uint8_t off3362;
    std::uint8_t off3363;
    std::uint8_t off3364;
    std::uint8_t off3365;
    std::uint8_t off3366;
    std::uint8_t off3367;
    std::uint8_t off3368;
    std::uint8_t off3369;
    std::uint8_t off3370;
    std::uint8_t off3371;
    std::uint8_t off3372;
    std::uint8_t off3373;
    std::uint8_t off3374;
    std::uint8_t off3375;
    std::uint8_t off3376;
    std::uint8_t off3377;
    std::uint8_t off3378;
    std::uint8_t off3379;
    std::uint8_t off3380;
    std::uint8_t off3381;
    std::uint8_t off3382;
    std::uint8_t off3383;
    std::uint8_t off3384;
    std::uint8_t off3385;
    std::uint8_t off3386;
    std::uint8_t off3387;
    std::uint8_t off3388;
    std::uint8_t off3389;
    std::uint8_t off3390;
    std::uint8_t off3391;
    std::uint8_t off3392;
    std::uint8_t off3393;
    std::uint8_t off3394;
    std::uint8_t off3395;
    std::uint8_t off3396;
    std::uint8_t off3397;
    std::uint8_t off3398;
    std::uint8_t off3399;
    std::uint8_t off3400;
    std::uint8_t off3401;
    std::uint8_t off3402;
    std::uint8_t off3403;
    std::uint8_t off3404;
    std::uint8_t off3405;
    std::uint8_t off3406;
    std::uint8_t off3407;
    std::uint8_t off3408;
    std::uint8_t off3409;
    std::uint8_t off3410;
    std::uint8_t off3411;
    std::uint8_t off3412;
    std::uint8_t off3413;
    std::uint8_t off3414;
    std::uint8_t off3415;
    std::uint8_t off3416;
    std::uint8_t off3417;
    std::uint8_t off3418;
    std::uint8_t off3419;
    std::uint8_t off3420;
    std::uint8_t off3421;
    std::uint8_t off3422;
    std::uint8_t off3423;
    std::uint8_t off3424;
    std::uint8_t off3425;
    std::uint8_t off3426;
    std::uint8_t off3427;
    std::uint8_t off3428;
    std::uint8_t off3429;
    std::uint8_t off3430;
    std::uint8_t off3431;
    std::uint8_t off3432;
    std::uint8_t off3433;
    std::uint8_t off3434;
    std::uint8_t off3435;
    std::uint8_t off3436;
    std::uint8_t off3437;
    std::uint8_t off3438;
    std::uint8_t off3439;
    std::uint8_t off3440;
    std::uint8_t off3441;
    std::uint8_t off3442;
    std::uint8_t off3443;
    std::uint8_t off3444;
    std::uint8_t off3445;
    std::uint8_t off3446;
    std::uint8_t off3447;
    std::uint8_t off3448;
    std::uint8_t off3449;
    std::uint8_t off3450;
    std::uint8_t off3451;
    std::uint8_t off3452;
    std::uint8_t off3453;
    std::uint8_t off3454;
    std::uint8_t off3455;
    std::uint8_t off3456;
    std::uint8_t off3457;
    std::uint8_t off3458;
    std::uint8_t off3459;
    std::uint8_t off3460;
    std::uint8_t off3461;
    std::uint8_t off3462;
    std::uint8_t off3463;
    std::uint8_t off3464;
    std::uint8_t off3465;
    std::uint8_t off3466;
    std::uint8_t off3467;
    std::uint8_t off3468;
    std::uint8_t off3469;
    std::uint8_t off3470;
    std::uint8_t off3471;
    std::uint8_t off3472;
    std::uint8_t off3473;
    std::uint8_t off3474;
    std::uint8_t off3475;
    std::uint8_t off3476;
    std::uint8_t off3477;
    std::uint8_t off3478;
    std::uint8_t off3479;
    std::uint8_t off3480;
    std::uint8_t off3481;
    std::uint8_t off3482;
    std::uint8_t off3483;
    std::uint8_t off3484;
    std::uint8_t off3485;
    std::uint8_t off3486;
    std::uint8_t off3487;
    std::uint8_t off3488;
    std::uint8_t off3489;
    std::uint8_t off3490;
    std::uint8_t off3491;
    std::uint8_t off3492;
    std::uint8_t off3493;
    std::uint8_t off3494;
    std::uint8_t off3495;
    std::uint8_t off3496;
    std::uint8_t off3497;
    std::uint8_t off3498;
    std::uint8_t off3499;
    std::uint8_t off3500;
    std::uint8_t off3501;
    std::uint8_t off3502;
    std::uint8_t off3503;
    std::uint8_t off3504;
    std::uint8_t off3505;
    std::uint8_t off3506;
    std::uint8_t off3507;
    std::uint8_t off3508;
    std::uint8_t off3509;
    std::uint8_t off3510;
    std::uint8_t off3511;
    std::uint8_t off3512;
    std::uint8_t off3513;
    std::uint8_t off3514;
    std::uint8_t off3515;
    std::uint8_t off3516;
    std::uint8_t off3517;
    std::uint8_t off3518;
    std::uint8_t off3519;
    std::uint8_t off3520;
    std::uint8_t off3521;
    std::uint8_t off3522;
    std::uint8_t off3523;
    std::uint8_t off3524;
    std::uint8_t off3525;
    std::uint8_t off3526;
    std::uint8_t off3527;
    std::uint8_t off3528;
    std::uint8_t off3529;
    std::uint8_t off3530;
    std::uint8_t off3531;
    std::uint8_t off3532;
    std::uint8_t off3533;
    std::uint8_t off3534;
    std::uint8_t off3535;
    std::uint8_t off3536;
    std::uint8_t off3537;
    std::uint8_t off3538;
    std::uint8_t off3539;
    std::uint8_t off3540;
    std::uint8_t off3541;
    std::uint8_t off3542;
    std::uint8_t off3543;
    std::uint8_t off3544;
    std::uint8_t off3545;
    std::uint8_t off3546;
    std::uint8_t off3547;
    std::uint8_t off3548;
    std::uint8_t off3549;
    std::uint8_t off3550;
    std::uint8_t off3551;
    std::uint8_t off3552;
    std::uint8_t off3553;
    std::uint8_t off3554;
    std::uint8_t off3555;
    std::uint8_t off3556;
    std::uint8_t off3557;
    std::uint8_t off3558;
    std::uint8_t off3559;
    std::uint8_t off3560;
    std::uint8_t off3561;
    std::uint8_t off3562;
    std::uint8_t off3563;
    std::uint8_t off3564;
    std::uint8_t off3565;
    std::uint8_t off3566;
    std::uint8_t off3567;
    std::uint8_t off3568;
    std::uint8_t off3569;
    std::uint8_t off3570;
    std::uint8_t off3571;
    std::uint8_t off3572;
    std::uint8_t off3573;
    std::uint8_t off3574;
    std::uint8_t off3575;
    std::uint8_t off3576;
    std::uint8_t off3577;
    std::uint8_t off3578;
    std::uint8_t off3579;
    std::uint8_t off3580;
    std::uint8_t off3581;
    std::uint8_t off3582;
    std::uint8_t off3583;
    std::uint8_t off3584;
    std::uint8_t off3585;
    std::uint8_t off3586;
    std::uint8_t off3587;
    std::uint8_t off3588;
    std::uint8_t off3589;
    std::uint8_t off3590;
    std::uint8_t off3591;
    std::uint8_t off3592;
    std::uint8_t off3593;
    std::uint8_t off3594;
    std::uint8_t off3595;
    std::uint8_t off3596;
    std::uint8_t off3597;
    std::uint8_t off3598;
    std::uint8_t off3599;
    std::uint8_t off3600;
    std::uint8_t off3601;
    std::uint8_t off3602;
    std::uint8_t off3603;
    std::uint8_t off3604;
    std::uint8_t off3605;
    std::uint8_t off3606;
    std::uint8_t off3607;
    std::uint8_t off3608;
    std::uint8_t off3609;
    std::uint8_t off3610;
    std::uint8_t off3611;
    std::uint8_t off3612;
    std::uint8_t off3613;
    std::uint8_t off3614;
    std::uint8_t off3615;
    std::uint8_t off3616;
    std::uint8_t off3617;
    std::uint8_t off3618;
    std::uint8_t off3619;
    std::uint8_t off3620;
    std::uint8_t off3621;
    std::uint8_t off3622;
    std::uint8_t off3623;
    std::uint8_t off3624;
    std::uint8_t off3625;
    std::uint8_t off3626;
    std::uint8_t off3627;
    std::uint8_t off3628;
    std::uint8_t off3629;
    std::uint8_t off3630;
    std::uint8_t off3631;
    std::uint8_t off3632;
    std::uint8_t off3633;
    std::uint8_t off3634;
    std::uint8_t off3635;
    std::uint8_t off3636;
    std::uint8_t off3637;
    std::uint8_t off3638;
    std::uint8_t off3639;
    std::uint8_t off3640;
    std::uint8_t off3641;
    std::uint8_t off3642;
    std::uint8_t off3643;
    std::uint8_t off3644;
    std::uint8_t off3645;
    std::uint8_t off3646;
    std::uint8_t off3647;
    std::uint8_t off3648;
    std::uint8_t off3649;
    std::uint8_t off3650;
    std::uint8_t off3651;
    std::uint8_t off3652;
    std::uint8_t off3653;
    std::uint8_t off3654;
    std::uint8_t off3655;
    std::uint8_t off3656;
    std::uint8_t off3657;
    std::uint8_t off3658;
    std::uint8_t off3659;
    std::uint8_t off3660;
    std::uint8_t off3661;
    std::uint8_t off3662;
    std::uint8_t off3663;
    std::uint8_t off3664;
    std::uint8_t off3665;
    std::uint8_t off3666;
    std::uint8_t off3667;
    std::uint8_t off3668;
    std::uint8_t off3669;
    std::uint8_t off3670;
    std::uint8_t off3671;
    std::uint8_t off3672;
    std::uint8_t off3673;
    std::uint8_t off3674;
    std::uint8_t off3675;
    std::uint8_t off3676;
    std::uint8_t off3677;
    std::uint8_t off3678;
    std::uint8_t off3679;
    std::uint8_t off3680;
    std::uint8_t off3681;
    std::uint8_t off3682;
    std::uint8_t off3683;
    std::uint8_t off3684;
    std::uint8_t off3685;
    std::uint8_t off3686;
    std::uint8_t off3687;
    std::uint8_t off3688;
    std::uint8_t off3689;
    std::uint8_t off3690;
    std::uint8_t off3691;
    std::uint8_t off3692;
    std::uint8_t off3693;
    std::uint8_t off3694;
    std::uint8_t off3695;
    std::uint8_t off3696;
    std::uint8_t off3697;
    std::uint8_t off3698;
    std::uint8_t off3699;
    std::uint8_t off3700;
    std::uint8_t off3701;
    std::uint8_t off3702;
    std::uint8_t off3703;
    std::uint8_t off3704;
    std::uint8_t off3705;
    std::uint8_t off3706;
    std::uint8_t off3707;
    std::uint8_t off3708;
    std::uint8_t off3709;
    std::uint8_t off3710;
    std::uint8_t off3711;
    std::uint8_t off3712;
    std::uint8_t off3713;
    std::uint8_t off3714;
    std::uint8_t off3715;
    std::uint8_t off3716;
    std::uint8_t off3717;
    std::uint8_t off3718;
    std::uint8_t off3719;
    std::uint8_t off3720;
    std::uint8_t off3721;
    std::uint8_t off3722;
    std::uint8_t off3723;
    std::uint8_t off3724;
    std::uint8_t off3725;
    std::uint8_t off3726;
    std::uint8_t off3727;
    std::uint8_t off3728;
    std::uint8_t off3729;
    std::uint8_t off3730;
    std::uint8_t off3731;
    std::uint8_t off3732;
    std::uint8_t off3733;
    std::uint8_t off3734;
    std::uint8_t off3735;
    std::uint8_t off3736;
    std::uint8_t off3737;
    std::uint8_t off3738;
    std::uint8_t off3739;
    std::uint8_t off3740;
    std::uint8_t off3741;
    std::uint8_t off3742;
    std::uint8_t off3743;
    std::uint8_t off3744;
    std::uint8_t off3745;
    std::uint8_t off3746;
    std::uint8_t off3747;
    std::uint8_t off3748;
    std::uint8_t off3749;
    std::uint8_t off3750;
    std::uint8_t off3751;
    std::uint8_t off3752;
    std::uint8_t off3753;
    std::uint8_t off3754;
    std::uint8_t off3755;
    std::uint8_t off3756;
    std::uint8_t off3757;
    std::uint8_t off3758;
    std::uint8_t off3759;
    std::uint8_t off3760;
    std::uint8_t off3761;
    std::uint8_t off3762;
    std::uint8_t off3763;
    std::uint8_t off3764;
    std::uint8_t off3765;
    std::uint8_t off3766;
    std::uint8_t off3767;
    std::uint8_t off3768;
    std::uint8_t off3769;
    std::uint8_t off3770;
    std::uint8_t off3771;
    std::uint8_t off3772;
    std::uint8_t off3773;
    std::uint8_t off3774;
    std::uint8_t off3775;
    std::uint8_t off3776;
    std::uint8_t off3777;
    std::uint8_t off3778;
    std::uint8_t off3779;
    std::uint8_t off3780;
    std::uint8_t off3781;
    std::uint8_t off3782;
    std::uint8_t off3783;
    std::uint8_t off3784;
    std::uint8_t off3785;
    std::uint8_t off3786;
    std::uint8_t off3787;
    std::uint8_t off3788;
    std::uint8_t off3789;
    std::uint8_t off3790;
    std::uint8_t off3791;
    std::uint8_t off3792;
    std::uint8_t off3793;
    std::uint8_t off3794;
    std::uint8_t off3795;
    std::uint8_t off3796;
    std::uint8_t off3797;
    std::uint8_t off3798;
    std::uint8_t off3799;
    std::uint8_t off3800;
    std::uint8_t off3801;
    std::uint8_t off3802;
    std::uint8_t off3803;
    std::uint8_t off3804;
    std::uint8_t off3805;
    std::uint8_t off3806;
    std::uint8_t off3807;
    std::uint8_t off3808;
    std::uint8_t off3809;
    std::uint8_t off3810;
    std::uint8_t off3811;
    std::uint8_t off3812;
    std::uint8_t off3813;
    std::uint8_t off3814;
    std::uint8_t off3815;
    std::uint8_t off3816;
    std::uint8_t off3817;
    std::uint8_t off3818;
    std::uint8_t off3819;
    std::uint8_t off3820;
    std::uint8_t off3821;
    std::uint8_t off3822;
    std::uint8_t off3823;
    std::uint8_t off3824;
    std::uint8_t off3825;
    std::uint8_t off3826;
    std::uint8_t off3827;
    std::uint8_t off3828;
    std::uint8_t off3829;
    std::uint8_t off3830;
    std::uint8_t off3831;
    std::uint8_t off3832;
    std::uint8_t off3833;
    std::uint8_t off3834;
    std::uint8_t off3835;
    std::uint8_t off3836;
    std::uint8_t off3837;
    std::uint8_t off3838;
    std::uint8_t off3839;
    std::uint8_t off3840;
    std::uint8_t off3841;
    std::uint8_t off3842;
    std::uint8_t off3843;
    std::uint8_t off3844;
    std::uint8_t off3845;
    std::uint8_t off3846;
    std::uint8_t off3847;
    std::uint8_t off3848;
    std::uint8_t off3849;
    std::uint8_t off3850;
    std::uint8_t off3851;
    std::uint8_t off3852;
    std::uint8_t off3853;
    std::uint8_t off3854;
    std::uint8_t off3855;
    std::uint8_t off3856;
    std::uint8_t off3857;
    std::uint8_t off3858;
    std::uint8_t off3859;
    std::uint8_t off3860;
    std::uint8_t off3861;
    std::uint8_t off3862;
    std::uint8_t off3863;
    std::uint8_t off3864;
    std::uint8_t off3865;
    std::uint8_t off3866;
    std::uint8_t off3867;
    std::uint8_t off3868;
    std::uint8_t off3869;
    std::uint8_t off3870;
    std::uint8_t off3871;
    std::uint8_t off3872;
    std::uint8_t off3873;
    std::uint8_t off3874;
    std::uint8_t off3875;
    std::uint8_t off3876;
    std::uint8_t off3877;
    std::uint8_t off3878;
    std::uint8_t off3879;
    std::uint8_t off3880;
    std::uint8_t off3881;
    std::uint8_t off3882;
    std::uint8_t off3883;
    std::uint8_t off3884;
    std::uint8_t off3885;
    std::uint8_t off3886;
    std::uint8_t off3887;
    std::uint8_t off3888;
    std::uint8_t off3889;
    std::uint8_t off3890;
    std::uint8_t off3891;
    std::uint8_t off3892;
    std::uint8_t off3893;
    std::uint8_t off3894;
    std::uint8_t off3895;
    std::uint8_t off3896;
    std::uint8_t off3897;
    std::uint8_t off3898;
    std::uint8_t off3899;
    std::uint8_t off3900;
    std::uint8_t off3901;
    std::uint8_t off3902;
    std::uint8_t off3903;
    std::uint8_t off3904;
    std::uint8_t off3905;
    std::uint8_t off3906;
    std::uint8_t off3907;
    std::uint8_t off3908;
    std::uint8_t off3909;
    std::uint8_t off3910;
    std::uint8_t off3911;
    std::uint8_t off3912;
    std::uint8_t off3913;
    std::uint8_t off3914;
    std::uint8_t off3915;
    std::uint8_t off3916;
    std::uint8_t off3917;
    std::uint8_t off3918;
    std::uint8_t off3919;
    std::uint8_t off3920;
    std::uint8_t off3921;
    std::uint8_t off3922;
    std::uint8_t off3923;
    std::uint8_t off3924;
    std::uint8_t off3925;
    std::uint8_t off3926;
    std::uint8_t off3927;
    std::uint8_t off3928;
    std::uint8_t off3929;
    std::uint8_t off3930;
    std::uint8_t off3931;
    std::uint8_t off3932;
    std::uint8_t off3933;
    std::uint8_t off3934;
    std::uint8_t off3935;
    std::uint8_t off3936;
    std::uint8_t off3937;
    std::uint8_t off3938;
    std::uint8_t off3939;
    std::uint8_t off3940;
    std::uint8_t off3941;
    std::uint8_t off3942;
    std::uint8_t off3943;
    std::uint8_t off3944;
    std::uint8_t off3945;
    std::uint8_t off3946;
    std::uint8_t off3947;
    std::uint8_t off3948;
    std::uint8_t off3949;
    std::uint8_t off3950;
    std::uint8_t off3951;
    std::uint8_t off3952;
    std::uint8_t off3953;
    std::uint8_t off3954;
    std::uint8_t off3955;
    std::uint8_t off3956;
    std::uint8_t off3957;
    std::uint8_t off3958;
    std::uint8_t off3959;
    std::uint8_t off3960;
    std::uint8_t off3961;
    std::uint8_t off3962;
    std::uint8_t off3963;
    std::uint8_t off3964;
    std::uint8_t off3965;
    std::uint8_t off3966;
    std::uint8_t off3967;
    std::uint8_t off3968;
    std::uint8_t off3969;
    std::uint8_t off3970;
    std::uint8_t off3971;
    std::uint8_t off3972;
    std::uint8_t off3973;
    std::uint8_t off3974;
    std::uint8_t off3975;
    std::uint8_t off3976;
    std::uint8_t off3977;
    std::uint8_t off3978;
    std::uint8_t off3979;
    std::uint8_t off3980;
    std::uint8_t off3981;
    std::uint8_t off3982;
    std::uint8_t off3983;
    std::uint8_t off3984;
    std::uint8_t off3985;
    std::uint8_t off3986;
    std::uint8_t off3987;
    std::uint8_t off3988;
    std::uint8_t off3989;
    std::uint8_t off3990;
    std::uint8_t off3991;
    std::uint8_t off3992;
    std::uint8_t off3993;
    std::uint8_t off3994;
    std::uint8_t off3995;
    std::uint8_t off3996;
    std::uint8_t off3997;
    std::uint8_t off3998;
    std::uint8_t off3999;
    std::uint8_t off4000;
    std::uint8_t off4001;
    std::uint8_t off4002;
    std::uint8_t off4003;
    std::uint8_t off4004;
    std::uint8_t off4005;
    std::uint8_t off4006;
    std::uint8_t off4007;
    std::uint8_t off4008;
    std::uint8_t off4009;
    std::uint8_t off4010;
    std::uint8_t off4011;
    std::uint8_t off4012;
    std::uint8_t off4013;
    std::uint8_t off4014;
    std::uint8_t off4015;
    std::uint8_t off4016;
    std::uint8_t off4017;
    std::uint8_t off4018;
    std::uint8_t off4019;
    std::uint8_t off4020;
    std::uint8_t off4021;
    std::uint8_t off4022;
    std::uint8_t off4023;
    std::uint8_t off4024;
    std::uint8_t off4025;
    std::uint8_t off4026;
    std::uint8_t off4027;
    std::uint8_t off4028;
    std::uint8_t off4029;
    std::uint8_t off4030;
    std::uint8_t off4031;
    std::uint8_t off4032;
    std::uint8_t off4033;
    std::uint8_t off4034;
    std::uint8_t off4035;
    std::uint8_t off4036;
    std::uint8_t off4037;
    std::uint8_t off4038;
    std::uint8_t off4039;
    std::uint8_t off4040;
    std::uint8_t off4041;
    std::uint8_t off4042;
    std::uint8_t off4043;
    std::uint8_t off4044;
    std::uint8_t off4045;
    std::uint8_t off4046;
    std::uint8_t off4047;
    std::uint8_t off4048;
    std::uint8_t off4049;
    std::uint8_t off4050;
    std::uint8_t off4051;
    std::uint8_t off4052;
    std::uint8_t off4053;
    std::uint8_t off4054;
    std::uint8_t off4055;
    std::uint8_t off4056;
    std::uint8_t off4057;
    std::uint8_t off4058;
    std::uint8_t off4059;
    std::uint8_t off4060;
    std::uint8_t off4061;
    std::uint8_t off4062;
    std::uint8_t off4063;
    std::uint8_t off4064;
    std::uint8_t off4065;
    std::uint8_t off4066;
    std::uint8_t off4067;
    std::uint8_t off4068;
    std::uint8_t off4069;
    std::uint8_t off4070;
    std::uint8_t off4071;
    std::uint8_t off4072;
    std::uint8_t off4073;
    std::uint8_t off4074;
    std::uint8_t off4075;
    std::uint8_t off4076;
    std::uint8_t off4077;
    std::uint8_t off4078;
    std::uint8_t off4079;
    std::uint8_t off4080;
    std::uint8_t off4081;
    std::uint8_t off4082;
    std::uint8_t off4083;
    std::uint8_t off4084;
    std::uint8_t off4085;
    std::uint8_t off4086;
    std::uint8_t off4087;
    std::uint8_t off4088;
    std::uint8_t off4089;
    std::uint8_t off4090;
    std::uint8_t off4091;
    std::uint8_t off4092;
    std::uint8_t off4093;
    std::uint8_t off4094;
    std::uint8_t off4095;
};

template <concepts::aggregate Ty>
static inline const auto& offsets_of() noexcept {
    constexpr auto count                                    = member_count_v<Ty>;
    auto& outline                                           = get_object_outline<Ty>();
    constexpr auto tuple                                    = object_to_tuple_view(outline);
    [[maybe_unused]] static std::array<size_t, count> array = { [&]<std::size_t... Is>(
                                                                    std::index_sequence<Is...>) {
        std::array<size_t, count> arr;
        ((arr[Is] = (size_t)((const char*)std::addressof(std::get<Is>(tuple)) -
                             (char*)(std::addressof(outline)))),
         ...);
        return arr;
    }(std::make_index_sequence<count>()) };
    return array;
}

template <concepts::aggregate Ty, typename... Args>
requires concepts::pure<Ty>
consteval static inline auto make_offset_tuple(const std::tuple<Args...>& tuple) noexcept {
    return std::make_tuple(
        static_cast<std::remove_cv_t<std::remove_pointer_t<Args>> Ty::*>(nullptr)...);
}

constexpr std::size_t offset_map_size = 256;

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define ATOM_CONCAT_(a, b) a##b
#define ATOM_CONCAT(a, b) ATOM_CONCAT_(a, b)
#define ATOM_SET_THE_OFFSET_(n, o) arr[n] = ATOM_CONCAT(&offset_helper::off, o);

inline namespace {

consteval static auto offset_map0() noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 0)
    ATOM_SET_THE_OFFSET_(1, 1)
    ATOM_SET_THE_OFFSET_(2, 2)
    ATOM_SET_THE_OFFSET_(3, 3)
    ATOM_SET_THE_OFFSET_(4, 4)
    ATOM_SET_THE_OFFSET_(5, 5)
    ATOM_SET_THE_OFFSET_(6, 6)
    ATOM_SET_THE_OFFSET_(7, 7)
    ATOM_SET_THE_OFFSET_(8, 8)
    ATOM_SET_THE_OFFSET_(9, 9)
    ATOM_SET_THE_OFFSET_(10, 10)
    ATOM_SET_THE_OFFSET_(11, 11)
    ATOM_SET_THE_OFFSET_(12, 12)
    ATOM_SET_THE_OFFSET_(13, 13)
    ATOM_SET_THE_OFFSET_(14, 14)
    ATOM_SET_THE_OFFSET_(15, 15)
    ATOM_SET_THE_OFFSET_(16, 16)
    ATOM_SET_THE_OFFSET_(17, 17)
    ATOM_SET_THE_OFFSET_(18, 18)
    ATOM_SET_THE_OFFSET_(19, 19)
    ATOM_SET_THE_OFFSET_(20, 20)
    ATOM_SET_THE_OFFSET_(21, 21)
    ATOM_SET_THE_OFFSET_(22, 22)
    ATOM_SET_THE_OFFSET_(23, 23)
    ATOM_SET_THE_OFFSET_(24, 24)
    ATOM_SET_THE_OFFSET_(25, 25)
    ATOM_SET_THE_OFFSET_(26, 26)
    ATOM_SET_THE_OFFSET_(27, 27)
    ATOM_SET_THE_OFFSET_(28, 28)
    ATOM_SET_THE_OFFSET_(29, 29)
    ATOM_SET_THE_OFFSET_(30, 30)
    ATOM_SET_THE_OFFSET_(31, 31)
    ATOM_SET_THE_OFFSET_(32, 32)
    ATOM_SET_THE_OFFSET_(33, 33)
    ATOM_SET_THE_OFFSET_(34, 34)
    ATOM_SET_THE_OFFSET_(35, 35)
    ATOM_SET_THE_OFFSET_(36, 36)
    ATOM_SET_THE_OFFSET_(37, 37)
    ATOM_SET_THE_OFFSET_(38, 38)
    ATOM_SET_THE_OFFSET_(39, 39)
    ATOM_SET_THE_OFFSET_(40, 40)
    ATOM_SET_THE_OFFSET_(41, 41)
    ATOM_SET_THE_OFFSET_(42, 42)
    ATOM_SET_THE_OFFSET_(43, 43)
    ATOM_SET_THE_OFFSET_(44, 44)
    ATOM_SET_THE_OFFSET_(45, 45)
    ATOM_SET_THE_OFFSET_(46, 46)
    ATOM_SET_THE_OFFSET_(47, 47)
    ATOM_SET_THE_OFFSET_(48, 48)
    ATOM_SET_THE_OFFSET_(49, 49)
    ATOM_SET_THE_OFFSET_(50, 50)
    ATOM_SET_THE_OFFSET_(51, 51)
    ATOM_SET_THE_OFFSET_(52, 52)
    ATOM_SET_THE_OFFSET_(53, 53)
    ATOM_SET_THE_OFFSET_(54, 54)
    ATOM_SET_THE_OFFSET_(55, 55)
    ATOM_SET_THE_OFFSET_(56, 56)
    ATOM_SET_THE_OFFSET_(57, 57)
    ATOM_SET_THE_OFFSET_(58, 58)
    ATOM_SET_THE_OFFSET_(59, 59)
    ATOM_SET_THE_OFFSET_(60, 60)
    ATOM_SET_THE_OFFSET_(61, 61)
    ATOM_SET_THE_OFFSET_(62, 62)
    ATOM_SET_THE_OFFSET_(63, 63)
    ATOM_SET_THE_OFFSET_(64, 64)
    ATOM_SET_THE_OFFSET_(65, 65)
    ATOM_SET_THE_OFFSET_(66, 66)
    ATOM_SET_THE_OFFSET_(67, 67)
    ATOM_SET_THE_OFFSET_(68, 68)
    ATOM_SET_THE_OFFSET_(69, 69)
    ATOM_SET_THE_OFFSET_(70, 70)
    ATOM_SET_THE_OFFSET_(71, 71)
    ATOM_SET_THE_OFFSET_(72, 72)
    ATOM_SET_THE_OFFSET_(73, 73)
    ATOM_SET_THE_OFFSET_(74, 74)
    ATOM_SET_THE_OFFSET_(75, 75)
    ATOM_SET_THE_OFFSET_(76, 76)
    ATOM_SET_THE_OFFSET_(77, 77)
    ATOM_SET_THE_OFFSET_(78, 78)
    ATOM_SET_THE_OFFSET_(79, 79)
    ATOM_SET_THE_OFFSET_(80, 80)
    ATOM_SET_THE_OFFSET_(81, 81)
    ATOM_SET_THE_OFFSET_(82, 82)
    ATOM_SET_THE_OFFSET_(83, 83)
    ATOM_SET_THE_OFFSET_(84, 84)
    ATOM_SET_THE_OFFSET_(85, 85)
    ATOM_SET_THE_OFFSET_(86, 86)
    ATOM_SET_THE_OFFSET_(87, 87)
    ATOM_SET_THE_OFFSET_(88, 88)
    ATOM_SET_THE_OFFSET_(89, 89)
    ATOM_SET_THE_OFFSET_(90, 90)
    ATOM_SET_THE_OFFSET_(91, 91)
    ATOM_SET_THE_OFFSET_(92, 92)
    ATOM_SET_THE_OFFSET_(93, 93)
    ATOM_SET_THE_OFFSET_(94, 94)
    ATOM_SET_THE_OFFSET_(95, 95)
    ATOM_SET_THE_OFFSET_(96, 96)
    ATOM_SET_THE_OFFSET_(97, 97)
    ATOM_SET_THE_OFFSET_(98, 98)
    ATOM_SET_THE_OFFSET_(99, 99)
    ATOM_SET_THE_OFFSET_(100, 100)
    ATOM_SET_THE_OFFSET_(101, 101)
    ATOM_SET_THE_OFFSET_(102, 102)
    ATOM_SET_THE_OFFSET_(103, 103)
    ATOM_SET_THE_OFFSET_(104, 104)
    ATOM_SET_THE_OFFSET_(105, 105)
    ATOM_SET_THE_OFFSET_(106, 106)
    ATOM_SET_THE_OFFSET_(107, 107)
    ATOM_SET_THE_OFFSET_(108, 108)
    ATOM_SET_THE_OFFSET_(109, 109)
    ATOM_SET_THE_OFFSET_(110, 110)
    ATOM_SET_THE_OFFSET_(111, 111)
    ATOM_SET_THE_OFFSET_(112, 112)
    ATOM_SET_THE_OFFSET_(113, 113)
    ATOM_SET_THE_OFFSET_(114, 114)
    ATOM_SET_THE_OFFSET_(115, 115)
    ATOM_SET_THE_OFFSET_(116, 116)
    ATOM_SET_THE_OFFSET_(117, 117)
    ATOM_SET_THE_OFFSET_(118, 118)
    ATOM_SET_THE_OFFSET_(119, 119)
    ATOM_SET_THE_OFFSET_(120, 120)
    ATOM_SET_THE_OFFSET_(121, 121)
    ATOM_SET_THE_OFFSET_(122, 122)
    ATOM_SET_THE_OFFSET_(123, 123)
    ATOM_SET_THE_OFFSET_(124, 124)
    ATOM_SET_THE_OFFSET_(125, 125)
    ATOM_SET_THE_OFFSET_(126, 126)
    ATOM_SET_THE_OFFSET_(127, 127)
    ATOM_SET_THE_OFFSET_(128, 128)
    ATOM_SET_THE_OFFSET_(129, 129)
    ATOM_SET_THE_OFFSET_(130, 130)
    ATOM_SET_THE_OFFSET_(131, 131)
    ATOM_SET_THE_OFFSET_(132, 132)
    ATOM_SET_THE_OFFSET_(133, 133)
    ATOM_SET_THE_OFFSET_(134, 134)
    ATOM_SET_THE_OFFSET_(135, 135)
    ATOM_SET_THE_OFFSET_(136, 136)
    ATOM_SET_THE_OFFSET_(137, 137)
    ATOM_SET_THE_OFFSET_(138, 138)
    ATOM_SET_THE_OFFSET_(139, 139)
    ATOM_SET_THE_OFFSET_(140, 140)
    ATOM_SET_THE_OFFSET_(141, 141)
    ATOM_SET_THE_OFFSET_(142, 142)
    ATOM_SET_THE_OFFSET_(143, 143)
    ATOM_SET_THE_OFFSET_(144, 144)
    ATOM_SET_THE_OFFSET_(145, 145)
    ATOM_SET_THE_OFFSET_(146, 146)
    ATOM_SET_THE_OFFSET_(147, 147)
    ATOM_SET_THE_OFFSET_(148, 148)
    ATOM_SET_THE_OFFSET_(149, 149)
    ATOM_SET_THE_OFFSET_(150, 150)
    ATOM_SET_THE_OFFSET_(151, 151)
    ATOM_SET_THE_OFFSET_(152, 152)
    ATOM_SET_THE_OFFSET_(153, 153)
    ATOM_SET_THE_OFFSET_(154, 154)
    ATOM_SET_THE_OFFSET_(155, 155)
    ATOM_SET_THE_OFFSET_(156, 156)
    ATOM_SET_THE_OFFSET_(157, 157)
    ATOM_SET_THE_OFFSET_(158, 158)
    ATOM_SET_THE_OFFSET_(159, 159)
    ATOM_SET_THE_OFFSET_(160, 160)
    ATOM_SET_THE_OFFSET_(161, 161)
    ATOM_SET_THE_OFFSET_(162, 162)
    ATOM_SET_THE_OFFSET_(163, 163)
    ATOM_SET_THE_OFFSET_(164, 164)
    ATOM_SET_THE_OFFSET_(165, 165)
    ATOM_SET_THE_OFFSET_(166, 166)
    ATOM_SET_THE_OFFSET_(167, 167)
    ATOM_SET_THE_OFFSET_(168, 168)
    ATOM_SET_THE_OFFSET_(169, 169)
    ATOM_SET_THE_OFFSET_(170, 170)
    ATOM_SET_THE_OFFSET_(171, 171)
    ATOM_SET_THE_OFFSET_(172, 172)
    ATOM_SET_THE_OFFSET_(173, 173)
    ATOM_SET_THE_OFFSET_(174, 174)
    ATOM_SET_THE_OFFSET_(175, 175)
    ATOM_SET_THE_OFFSET_(176, 176)
    ATOM_SET_THE_OFFSET_(177, 177)
    ATOM_SET_THE_OFFSET_(178, 178)
    ATOM_SET_THE_OFFSET_(179, 179)
    ATOM_SET_THE_OFFSET_(180, 180)
    ATOM_SET_THE_OFFSET_(181, 181)
    ATOM_SET_THE_OFFSET_(182, 182)
    ATOM_SET_THE_OFFSET_(183, 183)
    ATOM_SET_THE_OFFSET_(184, 184)
    ATOM_SET_THE_OFFSET_(185, 185)
    ATOM_SET_THE_OFFSET_(186, 186)
    ATOM_SET_THE_OFFSET_(187, 187)
    ATOM_SET_THE_OFFSET_(188, 188)
    ATOM_SET_THE_OFFSET_(189, 189)
    ATOM_SET_THE_OFFSET_(190, 190)
    ATOM_SET_THE_OFFSET_(191, 191)
    ATOM_SET_THE_OFFSET_(192, 192)
    ATOM_SET_THE_OFFSET_(193, 193)
    ATOM_SET_THE_OFFSET_(194, 194)
    ATOM_SET_THE_OFFSET_(195, 195)
    ATOM_SET_THE_OFFSET_(196, 196)
    ATOM_SET_THE_OFFSET_(197, 197)
    ATOM_SET_THE_OFFSET_(198, 198)
    ATOM_SET_THE_OFFSET_(199, 199)
    ATOM_SET_THE_OFFSET_(200, 200)
    ATOM_SET_THE_OFFSET_(201, 201)
    ATOM_SET_THE_OFFSET_(202, 202)
    ATOM_SET_THE_OFFSET_(203, 203)
    ATOM_SET_THE_OFFSET_(204, 204)
    ATOM_SET_THE_OFFSET_(205, 205)
    ATOM_SET_THE_OFFSET_(206, 206)
    ATOM_SET_THE_OFFSET_(207, 207)
    ATOM_SET_THE_OFFSET_(208, 208)
    ATOM_SET_THE_OFFSET_(209, 209)
    ATOM_SET_THE_OFFSET_(210, 210)
    ATOM_SET_THE_OFFSET_(211, 211)
    ATOM_SET_THE_OFFSET_(212, 212)
    ATOM_SET_THE_OFFSET_(213, 213)
    ATOM_SET_THE_OFFSET_(214, 214)
    ATOM_SET_THE_OFFSET_(215, 215)
    ATOM_SET_THE_OFFSET_(216, 216)
    ATOM_SET_THE_OFFSET_(217, 217)
    ATOM_SET_THE_OFFSET_(218, 218)
    ATOM_SET_THE_OFFSET_(219, 219)
    ATOM_SET_THE_OFFSET_(220, 220)
    ATOM_SET_THE_OFFSET_(221, 221)
    ATOM_SET_THE_OFFSET_(222, 222)
    ATOM_SET_THE_OFFSET_(223, 223)
    ATOM_SET_THE_OFFSET_(224, 224)
    ATOM_SET_THE_OFFSET_(225, 225)
    ATOM_SET_THE_OFFSET_(226, 226)
    ATOM_SET_THE_OFFSET_(227, 227)
    ATOM_SET_THE_OFFSET_(228, 228)
    ATOM_SET_THE_OFFSET_(229, 229)
    ATOM_SET_THE_OFFSET_(230, 230)
    ATOM_SET_THE_OFFSET_(231, 231)
    ATOM_SET_THE_OFFSET_(232, 232)
    ATOM_SET_THE_OFFSET_(233, 233)
    ATOM_SET_THE_OFFSET_(234, 234)
    ATOM_SET_THE_OFFSET_(235, 235)
    ATOM_SET_THE_OFFSET_(236, 236)
    ATOM_SET_THE_OFFSET_(237, 237)
    ATOM_SET_THE_OFFSET_(238, 238)
    ATOM_SET_THE_OFFSET_(239, 239)
    ATOM_SET_THE_OFFSET_(240, 240)
    ATOM_SET_THE_OFFSET_(241, 241)
    ATOM_SET_THE_OFFSET_(242, 242)
    ATOM_SET_THE_OFFSET_(243, 243)
    ATOM_SET_THE_OFFSET_(244, 244)
    ATOM_SET_THE_OFFSET_(245, 245)
    ATOM_SET_THE_OFFSET_(246, 246)
    ATOM_SET_THE_OFFSET_(247, 247)
    ATOM_SET_THE_OFFSET_(248, 248)
    ATOM_SET_THE_OFFSET_(249, 249)
    ATOM_SET_THE_OFFSET_(250, 250)
    ATOM_SET_THE_OFFSET_(251, 251)
    ATOM_SET_THE_OFFSET_(252, 252)
    ATOM_SET_THE_OFFSET_(253, 253)
    ATOM_SET_THE_OFFSET_(254, 254)
    ATOM_SET_THE_OFFSET_(255, 255)
    return arr;
}

consteval static auto offset_map1(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 256);
    ATOM_SET_THE_OFFSET_(1, 257);
    ATOM_SET_THE_OFFSET_(2, 258);
    ATOM_SET_THE_OFFSET_(3, 259);
    ATOM_SET_THE_OFFSET_(4, 260);
    ATOM_SET_THE_OFFSET_(5, 261);
    ATOM_SET_THE_OFFSET_(6, 262);
    ATOM_SET_THE_OFFSET_(7, 263);
    ATOM_SET_THE_OFFSET_(8, 264);
    ATOM_SET_THE_OFFSET_(9, 265);
    ATOM_SET_THE_OFFSET_(10, 266);
    ATOM_SET_THE_OFFSET_(11, 267);
    ATOM_SET_THE_OFFSET_(12, 268);
    ATOM_SET_THE_OFFSET_(13, 269);
    ATOM_SET_THE_OFFSET_(14, 270);
    ATOM_SET_THE_OFFSET_(15, 271);
    ATOM_SET_THE_OFFSET_(16, 272);
    ATOM_SET_THE_OFFSET_(17, 273);
    ATOM_SET_THE_OFFSET_(18, 274);
    ATOM_SET_THE_OFFSET_(19, 275);
    ATOM_SET_THE_OFFSET_(20, 276);
    ATOM_SET_THE_OFFSET_(21, 277);
    ATOM_SET_THE_OFFSET_(22, 278);
    ATOM_SET_THE_OFFSET_(23, 279);
    ATOM_SET_THE_OFFSET_(24, 280);
    ATOM_SET_THE_OFFSET_(25, 281);
    ATOM_SET_THE_OFFSET_(26, 282);
    ATOM_SET_THE_OFFSET_(27, 283);
    ATOM_SET_THE_OFFSET_(28, 284);
    ATOM_SET_THE_OFFSET_(29, 285);
    ATOM_SET_THE_OFFSET_(30, 286);
    ATOM_SET_THE_OFFSET_(31, 287);
    ATOM_SET_THE_OFFSET_(32, 288);
    ATOM_SET_THE_OFFSET_(33, 289);
    ATOM_SET_THE_OFFSET_(34, 290);
    ATOM_SET_THE_OFFSET_(35, 291);
    ATOM_SET_THE_OFFSET_(36, 292);
    ATOM_SET_THE_OFFSET_(37, 293);
    ATOM_SET_THE_OFFSET_(38, 294);
    ATOM_SET_THE_OFFSET_(39, 295);
    ATOM_SET_THE_OFFSET_(40, 296);
    ATOM_SET_THE_OFFSET_(41, 297);
    ATOM_SET_THE_OFFSET_(42, 298);
    ATOM_SET_THE_OFFSET_(43, 299);
    ATOM_SET_THE_OFFSET_(44, 300);
    ATOM_SET_THE_OFFSET_(45, 301);
    ATOM_SET_THE_OFFSET_(46, 302);
    ATOM_SET_THE_OFFSET_(47, 303);
    ATOM_SET_THE_OFFSET_(48, 304);
    ATOM_SET_THE_OFFSET_(49, 305);
    ATOM_SET_THE_OFFSET_(50, 306);
    ATOM_SET_THE_OFFSET_(51, 307);
    ATOM_SET_THE_OFFSET_(52, 308);
    ATOM_SET_THE_OFFSET_(53, 309);
    ATOM_SET_THE_OFFSET_(54, 310);
    ATOM_SET_THE_OFFSET_(55, 311);
    ATOM_SET_THE_OFFSET_(56, 312);
    ATOM_SET_THE_OFFSET_(57, 313);
    ATOM_SET_THE_OFFSET_(58, 314);
    ATOM_SET_THE_OFFSET_(59, 315);
    ATOM_SET_THE_OFFSET_(60, 316);
    ATOM_SET_THE_OFFSET_(61, 317);
    ATOM_SET_THE_OFFSET_(62, 318);
    ATOM_SET_THE_OFFSET_(63, 319);
    ATOM_SET_THE_OFFSET_(64, 320);
    ATOM_SET_THE_OFFSET_(65, 321);
    ATOM_SET_THE_OFFSET_(66, 322);
    ATOM_SET_THE_OFFSET_(67, 323);
    ATOM_SET_THE_OFFSET_(68, 324);
    ATOM_SET_THE_OFFSET_(69, 325);
    ATOM_SET_THE_OFFSET_(70, 326);
    ATOM_SET_THE_OFFSET_(71, 327);
    ATOM_SET_THE_OFFSET_(72, 328);
    ATOM_SET_THE_OFFSET_(73, 329);
    ATOM_SET_THE_OFFSET_(74, 330);
    ATOM_SET_THE_OFFSET_(75, 331);
    ATOM_SET_THE_OFFSET_(76, 332);
    ATOM_SET_THE_OFFSET_(77, 333);
    ATOM_SET_THE_OFFSET_(78, 334);
    ATOM_SET_THE_OFFSET_(79, 335);
    ATOM_SET_THE_OFFSET_(80, 336);
    ATOM_SET_THE_OFFSET_(81, 337);
    ATOM_SET_THE_OFFSET_(82, 338);
    ATOM_SET_THE_OFFSET_(83, 339);
    ATOM_SET_THE_OFFSET_(84, 340);
    ATOM_SET_THE_OFFSET_(85, 341);
    ATOM_SET_THE_OFFSET_(86, 342);
    ATOM_SET_THE_OFFSET_(87, 343);
    ATOM_SET_THE_OFFSET_(88, 344);
    ATOM_SET_THE_OFFSET_(89, 345);
    ATOM_SET_THE_OFFSET_(90, 346);
    ATOM_SET_THE_OFFSET_(91, 347);
    ATOM_SET_THE_OFFSET_(92, 348);
    ATOM_SET_THE_OFFSET_(93, 349);
    ATOM_SET_THE_OFFSET_(94, 350);
    ATOM_SET_THE_OFFSET_(95, 351);
    ATOM_SET_THE_OFFSET_(96, 352);
    ATOM_SET_THE_OFFSET_(97, 353);
    ATOM_SET_THE_OFFSET_(98, 354);
    ATOM_SET_THE_OFFSET_(99, 355);
    ATOM_SET_THE_OFFSET_(100, 356);
    ATOM_SET_THE_OFFSET_(101, 357);
    ATOM_SET_THE_OFFSET_(102, 358);
    ATOM_SET_THE_OFFSET_(103, 359);
    ATOM_SET_THE_OFFSET_(104, 360);
    ATOM_SET_THE_OFFSET_(105, 361);
    ATOM_SET_THE_OFFSET_(106, 362);
    ATOM_SET_THE_OFFSET_(107, 363);
    ATOM_SET_THE_OFFSET_(108, 364);
    ATOM_SET_THE_OFFSET_(109, 365);
    ATOM_SET_THE_OFFSET_(110, 366);
    ATOM_SET_THE_OFFSET_(111, 367);
    ATOM_SET_THE_OFFSET_(112, 368);
    ATOM_SET_THE_OFFSET_(113, 369);
    ATOM_SET_THE_OFFSET_(114, 370);
    ATOM_SET_THE_OFFSET_(115, 371);
    ATOM_SET_THE_OFFSET_(116, 372);
    ATOM_SET_THE_OFFSET_(117, 373);
    ATOM_SET_THE_OFFSET_(118, 374);
    ATOM_SET_THE_OFFSET_(119, 375);
    ATOM_SET_THE_OFFSET_(120, 376);
    ATOM_SET_THE_OFFSET_(121, 377);
    ATOM_SET_THE_OFFSET_(122, 378);
    ATOM_SET_THE_OFFSET_(123, 379);
    ATOM_SET_THE_OFFSET_(124, 380);
    ATOM_SET_THE_OFFSET_(125, 381);
    ATOM_SET_THE_OFFSET_(126, 382);
    ATOM_SET_THE_OFFSET_(127, 383);
    ATOM_SET_THE_OFFSET_(128, 384);
    ATOM_SET_THE_OFFSET_(129, 385);
    ATOM_SET_THE_OFFSET_(130, 386);
    ATOM_SET_THE_OFFSET_(131, 387);
    ATOM_SET_THE_OFFSET_(132, 388);
    ATOM_SET_THE_OFFSET_(133, 389);
    ATOM_SET_THE_OFFSET_(134, 390);
    ATOM_SET_THE_OFFSET_(135, 391);
    ATOM_SET_THE_OFFSET_(136, 392);
    ATOM_SET_THE_OFFSET_(137, 393);
    ATOM_SET_THE_OFFSET_(138, 394);
    ATOM_SET_THE_OFFSET_(139, 395);
    ATOM_SET_THE_OFFSET_(140, 396);
    ATOM_SET_THE_OFFSET_(141, 397);
    ATOM_SET_THE_OFFSET_(142, 398);
    ATOM_SET_THE_OFFSET_(143, 399);
    ATOM_SET_THE_OFFSET_(144, 400);
    ATOM_SET_THE_OFFSET_(145, 401);
    ATOM_SET_THE_OFFSET_(146, 402);
    ATOM_SET_THE_OFFSET_(147, 403);
    ATOM_SET_THE_OFFSET_(148, 404);
    ATOM_SET_THE_OFFSET_(149, 405);
    ATOM_SET_THE_OFFSET_(150, 406);
    ATOM_SET_THE_OFFSET_(151, 407);
    ATOM_SET_THE_OFFSET_(152, 408);
    ATOM_SET_THE_OFFSET_(153, 409);
    ATOM_SET_THE_OFFSET_(154, 410);
    ATOM_SET_THE_OFFSET_(155, 411);
    ATOM_SET_THE_OFFSET_(156, 412);
    ATOM_SET_THE_OFFSET_(157, 413);
    ATOM_SET_THE_OFFSET_(158, 414);
    ATOM_SET_THE_OFFSET_(159, 415);
    ATOM_SET_THE_OFFSET_(160, 416);
    ATOM_SET_THE_OFFSET_(161, 417);
    ATOM_SET_THE_OFFSET_(162, 418);
    ATOM_SET_THE_OFFSET_(163, 419);
    ATOM_SET_THE_OFFSET_(164, 420);
    ATOM_SET_THE_OFFSET_(165, 421);
    ATOM_SET_THE_OFFSET_(166, 422);
    ATOM_SET_THE_OFFSET_(167, 423);
    ATOM_SET_THE_OFFSET_(168, 424);
    ATOM_SET_THE_OFFSET_(169, 425);
    ATOM_SET_THE_OFFSET_(170, 426);
    ATOM_SET_THE_OFFSET_(171, 427);
    ATOM_SET_THE_OFFSET_(172, 428);
    ATOM_SET_THE_OFFSET_(173, 429);
    ATOM_SET_THE_OFFSET_(174, 430);
    ATOM_SET_THE_OFFSET_(175, 431);
    ATOM_SET_THE_OFFSET_(176, 432);
    ATOM_SET_THE_OFFSET_(177, 433);
    ATOM_SET_THE_OFFSET_(178, 434);
    ATOM_SET_THE_OFFSET_(179, 435);
    ATOM_SET_THE_OFFSET_(180, 436);
    ATOM_SET_THE_OFFSET_(181, 437);
    ATOM_SET_THE_OFFSET_(182, 438);
    ATOM_SET_THE_OFFSET_(183, 439);
    ATOM_SET_THE_OFFSET_(184, 440);
    ATOM_SET_THE_OFFSET_(185, 441);
    ATOM_SET_THE_OFFSET_(186, 442);
    ATOM_SET_THE_OFFSET_(187, 443);
    ATOM_SET_THE_OFFSET_(188, 444);
    ATOM_SET_THE_OFFSET_(189, 445);
    ATOM_SET_THE_OFFSET_(190, 446);
    ATOM_SET_THE_OFFSET_(191, 447);
    ATOM_SET_THE_OFFSET_(192, 448);
    ATOM_SET_THE_OFFSET_(193, 449);
    ATOM_SET_THE_OFFSET_(194, 450);
    ATOM_SET_THE_OFFSET_(195, 451);
    ATOM_SET_THE_OFFSET_(196, 452);
    ATOM_SET_THE_OFFSET_(197, 453);
    ATOM_SET_THE_OFFSET_(198, 454);
    ATOM_SET_THE_OFFSET_(199, 455);
    ATOM_SET_THE_OFFSET_(200, 456);
    ATOM_SET_THE_OFFSET_(201, 457);
    ATOM_SET_THE_OFFSET_(202, 458);
    ATOM_SET_THE_OFFSET_(203, 459);
    ATOM_SET_THE_OFFSET_(204, 460);
    ATOM_SET_THE_OFFSET_(205, 461);
    ATOM_SET_THE_OFFSET_(206, 462);
    ATOM_SET_THE_OFFSET_(207, 463);
    ATOM_SET_THE_OFFSET_(208, 464);
    ATOM_SET_THE_OFFSET_(209, 465);
    ATOM_SET_THE_OFFSET_(210, 466);
    ATOM_SET_THE_OFFSET_(211, 467);
    ATOM_SET_THE_OFFSET_(212, 468);
    ATOM_SET_THE_OFFSET_(213, 469);
    ATOM_SET_THE_OFFSET_(214, 470);
    ATOM_SET_THE_OFFSET_(215, 471);
    ATOM_SET_THE_OFFSET_(216, 472);
    ATOM_SET_THE_OFFSET_(217, 473);
    ATOM_SET_THE_OFFSET_(218, 474);
    ATOM_SET_THE_OFFSET_(219, 475);
    ATOM_SET_THE_OFFSET_(220, 476);
    ATOM_SET_THE_OFFSET_(221, 477);
    ATOM_SET_THE_OFFSET_(222, 478);
    ATOM_SET_THE_OFFSET_(223, 479);
    ATOM_SET_THE_OFFSET_(224, 480);
    ATOM_SET_THE_OFFSET_(225, 481);
    ATOM_SET_THE_OFFSET_(226, 482);
    ATOM_SET_THE_OFFSET_(227, 483);
    ATOM_SET_THE_OFFSET_(228, 484);
    ATOM_SET_THE_OFFSET_(229, 485);
    ATOM_SET_THE_OFFSET_(230, 486);
    ATOM_SET_THE_OFFSET_(231, 487);
    ATOM_SET_THE_OFFSET_(232, 488);
    ATOM_SET_THE_OFFSET_(233, 489);
    ATOM_SET_THE_OFFSET_(234, 490);
    ATOM_SET_THE_OFFSET_(235, 491);
    ATOM_SET_THE_OFFSET_(236, 492);
    ATOM_SET_THE_OFFSET_(237, 493);
    ATOM_SET_THE_OFFSET_(238, 494);
    ATOM_SET_THE_OFFSET_(239, 495);
    ATOM_SET_THE_OFFSET_(240, 496);
    ATOM_SET_THE_OFFSET_(241, 497);
    ATOM_SET_THE_OFFSET_(242, 498);
    ATOM_SET_THE_OFFSET_(243, 499);
    ATOM_SET_THE_OFFSET_(244, 500);
    ATOM_SET_THE_OFFSET_(245, 501);
    ATOM_SET_THE_OFFSET_(246, 502);
    ATOM_SET_THE_OFFSET_(247, 503);
    ATOM_SET_THE_OFFSET_(248, 504);
    ATOM_SET_THE_OFFSET_(249, 505);
    ATOM_SET_THE_OFFSET_(250, 506);
    ATOM_SET_THE_OFFSET_(251, 507);
    ATOM_SET_THE_OFFSET_(252, 508);
    ATOM_SET_THE_OFFSET_(253, 509);
    ATOM_SET_THE_OFFSET_(254, 510);
    ATOM_SET_THE_OFFSET_(255, 511);
    return arr;
}

consteval static auto offset_map2(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 512)
    ATOM_SET_THE_OFFSET_(1, 513)
    ATOM_SET_THE_OFFSET_(2, 514)
    ATOM_SET_THE_OFFSET_(3, 515)
    ATOM_SET_THE_OFFSET_(4, 516)
    ATOM_SET_THE_OFFSET_(5, 517)
    ATOM_SET_THE_OFFSET_(6, 518)
    ATOM_SET_THE_OFFSET_(7, 519)
    ATOM_SET_THE_OFFSET_(8, 520)
    ATOM_SET_THE_OFFSET_(9, 521)
    ATOM_SET_THE_OFFSET_(10, 522)
    ATOM_SET_THE_OFFSET_(11, 523)
    ATOM_SET_THE_OFFSET_(12, 524)
    ATOM_SET_THE_OFFSET_(13, 525)
    ATOM_SET_THE_OFFSET_(14, 526)
    ATOM_SET_THE_OFFSET_(15, 527)
    ATOM_SET_THE_OFFSET_(16, 528)
    ATOM_SET_THE_OFFSET_(17, 529)
    ATOM_SET_THE_OFFSET_(18, 530)
    ATOM_SET_THE_OFFSET_(19, 531)
    ATOM_SET_THE_OFFSET_(20, 532)
    ATOM_SET_THE_OFFSET_(21, 533)
    ATOM_SET_THE_OFFSET_(22, 534)
    ATOM_SET_THE_OFFSET_(23, 535)
    ATOM_SET_THE_OFFSET_(24, 536)
    ATOM_SET_THE_OFFSET_(25, 537)
    ATOM_SET_THE_OFFSET_(26, 538)
    ATOM_SET_THE_OFFSET_(27, 539)
    ATOM_SET_THE_OFFSET_(28, 540)
    ATOM_SET_THE_OFFSET_(29, 541)
    ATOM_SET_THE_OFFSET_(30, 542)
    ATOM_SET_THE_OFFSET_(31, 543)
    ATOM_SET_THE_OFFSET_(32, 544)
    ATOM_SET_THE_OFFSET_(33, 545)
    ATOM_SET_THE_OFFSET_(34, 546)
    ATOM_SET_THE_OFFSET_(35, 547)
    ATOM_SET_THE_OFFSET_(36, 548)
    ATOM_SET_THE_OFFSET_(37, 549)
    ATOM_SET_THE_OFFSET_(38, 550)
    ATOM_SET_THE_OFFSET_(39, 551)
    ATOM_SET_THE_OFFSET_(40, 552)
    ATOM_SET_THE_OFFSET_(41, 553)
    ATOM_SET_THE_OFFSET_(42, 554)
    ATOM_SET_THE_OFFSET_(43, 555)
    ATOM_SET_THE_OFFSET_(44, 556)
    ATOM_SET_THE_OFFSET_(45, 557)
    ATOM_SET_THE_OFFSET_(46, 558)
    ATOM_SET_THE_OFFSET_(47, 559)
    ATOM_SET_THE_OFFSET_(48, 560)
    ATOM_SET_THE_OFFSET_(49, 561)
    ATOM_SET_THE_OFFSET_(50, 562)
    ATOM_SET_THE_OFFSET_(51, 563)
    ATOM_SET_THE_OFFSET_(52, 564)
    ATOM_SET_THE_OFFSET_(53, 565)
    ATOM_SET_THE_OFFSET_(54, 566)
    ATOM_SET_THE_OFFSET_(55, 567)
    ATOM_SET_THE_OFFSET_(56, 568)
    ATOM_SET_THE_OFFSET_(57, 569)
    ATOM_SET_THE_OFFSET_(58, 570)
    ATOM_SET_THE_OFFSET_(59, 571)
    ATOM_SET_THE_OFFSET_(60, 572)
    ATOM_SET_THE_OFFSET_(61, 573)
    ATOM_SET_THE_OFFSET_(62, 574)
    ATOM_SET_THE_OFFSET_(63, 575)
    ATOM_SET_THE_OFFSET_(64, 576)
    ATOM_SET_THE_OFFSET_(65, 577)
    ATOM_SET_THE_OFFSET_(66, 578)
    ATOM_SET_THE_OFFSET_(67, 579)
    ATOM_SET_THE_OFFSET_(68, 580)
    ATOM_SET_THE_OFFSET_(69, 581)
    ATOM_SET_THE_OFFSET_(70, 582)
    ATOM_SET_THE_OFFSET_(71, 583)
    ATOM_SET_THE_OFFSET_(72, 584)
    ATOM_SET_THE_OFFSET_(73, 585)
    ATOM_SET_THE_OFFSET_(74, 586)
    ATOM_SET_THE_OFFSET_(75, 587)
    ATOM_SET_THE_OFFSET_(76, 588)
    ATOM_SET_THE_OFFSET_(77, 589)
    ATOM_SET_THE_OFFSET_(78, 590)
    ATOM_SET_THE_OFFSET_(79, 591)
    ATOM_SET_THE_OFFSET_(80, 592)
    ATOM_SET_THE_OFFSET_(81, 593)
    ATOM_SET_THE_OFFSET_(82, 594)
    ATOM_SET_THE_OFFSET_(83, 595)
    ATOM_SET_THE_OFFSET_(84, 596)
    ATOM_SET_THE_OFFSET_(85, 597)
    ATOM_SET_THE_OFFSET_(86, 598)
    ATOM_SET_THE_OFFSET_(87, 599)
    ATOM_SET_THE_OFFSET_(88, 600)
    ATOM_SET_THE_OFFSET_(89, 601)
    ATOM_SET_THE_OFFSET_(90, 602)
    ATOM_SET_THE_OFFSET_(91, 603)
    ATOM_SET_THE_OFFSET_(92, 604)
    ATOM_SET_THE_OFFSET_(93, 605)
    ATOM_SET_THE_OFFSET_(94, 606)
    ATOM_SET_THE_OFFSET_(95, 607)
    ATOM_SET_THE_OFFSET_(96, 608)
    ATOM_SET_THE_OFFSET_(97, 609)
    ATOM_SET_THE_OFFSET_(98, 610)
    ATOM_SET_THE_OFFSET_(99, 611)
    ATOM_SET_THE_OFFSET_(100, 612)
    ATOM_SET_THE_OFFSET_(101, 613)
    ATOM_SET_THE_OFFSET_(102, 614)
    ATOM_SET_THE_OFFSET_(103, 615)
    ATOM_SET_THE_OFFSET_(104, 616)
    ATOM_SET_THE_OFFSET_(105, 617)
    ATOM_SET_THE_OFFSET_(106, 618)
    ATOM_SET_THE_OFFSET_(107, 619)
    ATOM_SET_THE_OFFSET_(108, 620)
    ATOM_SET_THE_OFFSET_(109, 621)
    ATOM_SET_THE_OFFSET_(110, 622)
    ATOM_SET_THE_OFFSET_(111, 623)
    ATOM_SET_THE_OFFSET_(112, 624)
    ATOM_SET_THE_OFFSET_(113, 625)
    ATOM_SET_THE_OFFSET_(114, 626)
    ATOM_SET_THE_OFFSET_(115, 627)
    ATOM_SET_THE_OFFSET_(116, 628)
    ATOM_SET_THE_OFFSET_(117, 629)
    ATOM_SET_THE_OFFSET_(118, 630)
    ATOM_SET_THE_OFFSET_(119, 631)
    ATOM_SET_THE_OFFSET_(120, 632)
    ATOM_SET_THE_OFFSET_(121, 633)
    ATOM_SET_THE_OFFSET_(122, 634)
    ATOM_SET_THE_OFFSET_(123, 635)
    ATOM_SET_THE_OFFSET_(124, 636)
    ATOM_SET_THE_OFFSET_(125, 637)
    ATOM_SET_THE_OFFSET_(126, 638)
    ATOM_SET_THE_OFFSET_(127, 639)
    ATOM_SET_THE_OFFSET_(128, 640)
    ATOM_SET_THE_OFFSET_(129, 641)
    ATOM_SET_THE_OFFSET_(130, 642)
    ATOM_SET_THE_OFFSET_(131, 643)
    ATOM_SET_THE_OFFSET_(132, 644)
    ATOM_SET_THE_OFFSET_(133, 645)
    ATOM_SET_THE_OFFSET_(134, 646)
    ATOM_SET_THE_OFFSET_(135, 647)
    ATOM_SET_THE_OFFSET_(136, 648)
    ATOM_SET_THE_OFFSET_(137, 649)
    ATOM_SET_THE_OFFSET_(138, 650)
    ATOM_SET_THE_OFFSET_(139, 651)
    ATOM_SET_THE_OFFSET_(140, 652)
    ATOM_SET_THE_OFFSET_(141, 653)
    ATOM_SET_THE_OFFSET_(142, 654)
    ATOM_SET_THE_OFFSET_(143, 655)
    ATOM_SET_THE_OFFSET_(144, 656)
    ATOM_SET_THE_OFFSET_(145, 657)
    ATOM_SET_THE_OFFSET_(146, 658)
    ATOM_SET_THE_OFFSET_(147, 659)
    ATOM_SET_THE_OFFSET_(148, 660)
    ATOM_SET_THE_OFFSET_(149, 661)
    ATOM_SET_THE_OFFSET_(150, 662)
    ATOM_SET_THE_OFFSET_(151, 663)
    ATOM_SET_THE_OFFSET_(152, 664)
    ATOM_SET_THE_OFFSET_(153, 665)
    ATOM_SET_THE_OFFSET_(154, 666)
    ATOM_SET_THE_OFFSET_(155, 667)
    ATOM_SET_THE_OFFSET_(156, 668)
    ATOM_SET_THE_OFFSET_(157, 669)
    ATOM_SET_THE_OFFSET_(158, 670)
    ATOM_SET_THE_OFFSET_(159, 671)
    ATOM_SET_THE_OFFSET_(160, 672)
    ATOM_SET_THE_OFFSET_(161, 673)
    ATOM_SET_THE_OFFSET_(162, 674)
    ATOM_SET_THE_OFFSET_(163, 675)
    ATOM_SET_THE_OFFSET_(164, 676)
    ATOM_SET_THE_OFFSET_(165, 677)
    ATOM_SET_THE_OFFSET_(166, 678)
    ATOM_SET_THE_OFFSET_(167, 679)
    ATOM_SET_THE_OFFSET_(168, 680)
    ATOM_SET_THE_OFFSET_(169, 681)
    ATOM_SET_THE_OFFSET_(170, 682)
    ATOM_SET_THE_OFFSET_(171, 683)
    ATOM_SET_THE_OFFSET_(172, 684)
    ATOM_SET_THE_OFFSET_(173, 685)
    ATOM_SET_THE_OFFSET_(174, 686)
    ATOM_SET_THE_OFFSET_(175, 687)
    ATOM_SET_THE_OFFSET_(176, 688)
    ATOM_SET_THE_OFFSET_(177, 689)
    ATOM_SET_THE_OFFSET_(178, 690)
    ATOM_SET_THE_OFFSET_(179, 691)
    ATOM_SET_THE_OFFSET_(180, 692)
    ATOM_SET_THE_OFFSET_(181, 693)
    ATOM_SET_THE_OFFSET_(182, 694)
    ATOM_SET_THE_OFFSET_(183, 695)
    ATOM_SET_THE_OFFSET_(184, 696)
    ATOM_SET_THE_OFFSET_(185, 697)
    ATOM_SET_THE_OFFSET_(186, 698)
    ATOM_SET_THE_OFFSET_(187, 699)
    ATOM_SET_THE_OFFSET_(188, 700)
    ATOM_SET_THE_OFFSET_(189, 701)
    ATOM_SET_THE_OFFSET_(190, 702)
    ATOM_SET_THE_OFFSET_(191, 703)
    ATOM_SET_THE_OFFSET_(192, 704)
    ATOM_SET_THE_OFFSET_(193, 705)
    ATOM_SET_THE_OFFSET_(194, 706)
    ATOM_SET_THE_OFFSET_(195, 707)
    ATOM_SET_THE_OFFSET_(196, 708)
    ATOM_SET_THE_OFFSET_(197, 709)
    ATOM_SET_THE_OFFSET_(198, 710)
    ATOM_SET_THE_OFFSET_(199, 711)
    ATOM_SET_THE_OFFSET_(200, 712)
    ATOM_SET_THE_OFFSET_(201, 713)
    ATOM_SET_THE_OFFSET_(202, 714)
    ATOM_SET_THE_OFFSET_(203, 715)
    ATOM_SET_THE_OFFSET_(204, 716)
    ATOM_SET_THE_OFFSET_(205, 717)
    ATOM_SET_THE_OFFSET_(206, 718)
    ATOM_SET_THE_OFFSET_(207, 719)
    ATOM_SET_THE_OFFSET_(208, 720)
    ATOM_SET_THE_OFFSET_(209, 721)
    ATOM_SET_THE_OFFSET_(210, 722)
    ATOM_SET_THE_OFFSET_(211, 723)
    ATOM_SET_THE_OFFSET_(212, 724)
    ATOM_SET_THE_OFFSET_(213, 725)
    ATOM_SET_THE_OFFSET_(214, 726)
    ATOM_SET_THE_OFFSET_(215, 727)
    ATOM_SET_THE_OFFSET_(216, 728)
    ATOM_SET_THE_OFFSET_(217, 729)
    ATOM_SET_THE_OFFSET_(218, 730)
    ATOM_SET_THE_OFFSET_(219, 731)
    ATOM_SET_THE_OFFSET_(220, 732)
    ATOM_SET_THE_OFFSET_(221, 733)
    ATOM_SET_THE_OFFSET_(222, 734)
    ATOM_SET_THE_OFFSET_(223, 735)
    ATOM_SET_THE_OFFSET_(224, 736)
    ATOM_SET_THE_OFFSET_(225, 737)
    ATOM_SET_THE_OFFSET_(226, 738)
    ATOM_SET_THE_OFFSET_(227, 739)
    ATOM_SET_THE_OFFSET_(228, 740)
    ATOM_SET_THE_OFFSET_(229, 741)
    ATOM_SET_THE_OFFSET_(230, 742)
    ATOM_SET_THE_OFFSET_(231, 743)
    ATOM_SET_THE_OFFSET_(232, 744)
    ATOM_SET_THE_OFFSET_(233, 745)
    ATOM_SET_THE_OFFSET_(234, 746)
    ATOM_SET_THE_OFFSET_(235, 747)
    ATOM_SET_THE_OFFSET_(236, 748)
    ATOM_SET_THE_OFFSET_(237, 749)
    ATOM_SET_THE_OFFSET_(238, 750)
    ATOM_SET_THE_OFFSET_(239, 751)
    ATOM_SET_THE_OFFSET_(240, 752)
    ATOM_SET_THE_OFFSET_(241, 753)
    ATOM_SET_THE_OFFSET_(242, 754)
    ATOM_SET_THE_OFFSET_(243, 755)
    ATOM_SET_THE_OFFSET_(244, 756)
    ATOM_SET_THE_OFFSET_(245, 757)
    ATOM_SET_THE_OFFSET_(246, 758)
    ATOM_SET_THE_OFFSET_(247, 759)
    ATOM_SET_THE_OFFSET_(248, 760)
    ATOM_SET_THE_OFFSET_(249, 761)
    ATOM_SET_THE_OFFSET_(250, 762)
    ATOM_SET_THE_OFFSET_(251, 763)
    ATOM_SET_THE_OFFSET_(252, 764)
    ATOM_SET_THE_OFFSET_(253, 765)
    ATOM_SET_THE_OFFSET_(254, 766)
    ATOM_SET_THE_OFFSET_(255, 767)
    return arr;
}

consteval static auto offset_map3(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 768)
    ATOM_SET_THE_OFFSET_(1, 769)
    ATOM_SET_THE_OFFSET_(2, 770)
    ATOM_SET_THE_OFFSET_(3, 771)
    ATOM_SET_THE_OFFSET_(4, 772)
    ATOM_SET_THE_OFFSET_(5, 773)
    ATOM_SET_THE_OFFSET_(6, 774)
    ATOM_SET_THE_OFFSET_(7, 775)
    ATOM_SET_THE_OFFSET_(8, 776)
    ATOM_SET_THE_OFFSET_(9, 777)
    ATOM_SET_THE_OFFSET_(10, 778)
    ATOM_SET_THE_OFFSET_(11, 779)
    ATOM_SET_THE_OFFSET_(12, 780)
    ATOM_SET_THE_OFFSET_(13, 781)
    ATOM_SET_THE_OFFSET_(14, 782)
    ATOM_SET_THE_OFFSET_(15, 783)
    ATOM_SET_THE_OFFSET_(16, 784)
    ATOM_SET_THE_OFFSET_(17, 785)
    ATOM_SET_THE_OFFSET_(18, 786)
    ATOM_SET_THE_OFFSET_(19, 787)
    ATOM_SET_THE_OFFSET_(20, 788)
    ATOM_SET_THE_OFFSET_(21, 789)
    ATOM_SET_THE_OFFSET_(22, 790)
    ATOM_SET_THE_OFFSET_(23, 791)
    ATOM_SET_THE_OFFSET_(24, 792)
    ATOM_SET_THE_OFFSET_(25, 793)
    ATOM_SET_THE_OFFSET_(26, 794)
    ATOM_SET_THE_OFFSET_(27, 795)
    ATOM_SET_THE_OFFSET_(28, 796)
    ATOM_SET_THE_OFFSET_(29, 797)
    ATOM_SET_THE_OFFSET_(30, 798)
    ATOM_SET_THE_OFFSET_(31, 799)
    ATOM_SET_THE_OFFSET_(32, 800)
    ATOM_SET_THE_OFFSET_(33, 801)
    ATOM_SET_THE_OFFSET_(34, 802)
    ATOM_SET_THE_OFFSET_(35, 803)
    ATOM_SET_THE_OFFSET_(36, 804)
    ATOM_SET_THE_OFFSET_(37, 805)
    ATOM_SET_THE_OFFSET_(38, 806)
    ATOM_SET_THE_OFFSET_(39, 807)
    ATOM_SET_THE_OFFSET_(40, 808)
    ATOM_SET_THE_OFFSET_(41, 809)
    ATOM_SET_THE_OFFSET_(42, 810)
    ATOM_SET_THE_OFFSET_(43, 811)
    ATOM_SET_THE_OFFSET_(44, 812)
    ATOM_SET_THE_OFFSET_(45, 813)
    ATOM_SET_THE_OFFSET_(46, 814)
    ATOM_SET_THE_OFFSET_(47, 815)
    ATOM_SET_THE_OFFSET_(48, 816)
    ATOM_SET_THE_OFFSET_(49, 817)
    ATOM_SET_THE_OFFSET_(50, 818)
    ATOM_SET_THE_OFFSET_(51, 819)
    ATOM_SET_THE_OFFSET_(52, 820)
    ATOM_SET_THE_OFFSET_(53, 821)
    ATOM_SET_THE_OFFSET_(54, 822)
    ATOM_SET_THE_OFFSET_(55, 823)
    ATOM_SET_THE_OFFSET_(56, 824)
    ATOM_SET_THE_OFFSET_(57, 825)
    ATOM_SET_THE_OFFSET_(58, 826)
    ATOM_SET_THE_OFFSET_(59, 827)
    ATOM_SET_THE_OFFSET_(60, 828)
    ATOM_SET_THE_OFFSET_(61, 829)
    ATOM_SET_THE_OFFSET_(62, 830)
    ATOM_SET_THE_OFFSET_(63, 831)
    ATOM_SET_THE_OFFSET_(64, 832)
    ATOM_SET_THE_OFFSET_(65, 833)
    ATOM_SET_THE_OFFSET_(66, 834)
    ATOM_SET_THE_OFFSET_(67, 835)
    ATOM_SET_THE_OFFSET_(68, 836)
    ATOM_SET_THE_OFFSET_(69, 837)
    ATOM_SET_THE_OFFSET_(70, 838)
    ATOM_SET_THE_OFFSET_(71, 839)
    ATOM_SET_THE_OFFSET_(72, 840)
    ATOM_SET_THE_OFFSET_(73, 841)
    ATOM_SET_THE_OFFSET_(74, 842)
    ATOM_SET_THE_OFFSET_(75, 843)
    ATOM_SET_THE_OFFSET_(76, 844)
    ATOM_SET_THE_OFFSET_(77, 845)
    ATOM_SET_THE_OFFSET_(78, 846)
    ATOM_SET_THE_OFFSET_(79, 847)
    ATOM_SET_THE_OFFSET_(80, 848)
    ATOM_SET_THE_OFFSET_(81, 849)
    ATOM_SET_THE_OFFSET_(82, 850)
    ATOM_SET_THE_OFFSET_(83, 851)
    ATOM_SET_THE_OFFSET_(84, 852)
    ATOM_SET_THE_OFFSET_(85, 853)
    ATOM_SET_THE_OFFSET_(86, 854)
    ATOM_SET_THE_OFFSET_(87, 855)
    ATOM_SET_THE_OFFSET_(88, 856)
    ATOM_SET_THE_OFFSET_(89, 857)
    ATOM_SET_THE_OFFSET_(90, 858)
    ATOM_SET_THE_OFFSET_(91, 859)
    ATOM_SET_THE_OFFSET_(92, 860)
    ATOM_SET_THE_OFFSET_(93, 861)
    ATOM_SET_THE_OFFSET_(94, 862)
    ATOM_SET_THE_OFFSET_(95, 863)
    ATOM_SET_THE_OFFSET_(96, 864)
    ATOM_SET_THE_OFFSET_(97, 865)
    ATOM_SET_THE_OFFSET_(98, 866)
    ATOM_SET_THE_OFFSET_(99, 867)
    ATOM_SET_THE_OFFSET_(100, 868)
    ATOM_SET_THE_OFFSET_(101, 869)
    ATOM_SET_THE_OFFSET_(102, 870)
    ATOM_SET_THE_OFFSET_(103, 871)
    ATOM_SET_THE_OFFSET_(104, 872)
    ATOM_SET_THE_OFFSET_(105, 873)
    ATOM_SET_THE_OFFSET_(106, 874)
    ATOM_SET_THE_OFFSET_(107, 875)
    ATOM_SET_THE_OFFSET_(108, 876)
    ATOM_SET_THE_OFFSET_(109, 877)
    ATOM_SET_THE_OFFSET_(110, 878)
    ATOM_SET_THE_OFFSET_(111, 879)
    ATOM_SET_THE_OFFSET_(112, 880)
    ATOM_SET_THE_OFFSET_(113, 881)
    ATOM_SET_THE_OFFSET_(114, 882)
    ATOM_SET_THE_OFFSET_(115, 883)
    ATOM_SET_THE_OFFSET_(116, 884)
    ATOM_SET_THE_OFFSET_(117, 885)
    ATOM_SET_THE_OFFSET_(118, 886)
    ATOM_SET_THE_OFFSET_(119, 887)
    ATOM_SET_THE_OFFSET_(120, 888)
    ATOM_SET_THE_OFFSET_(121, 889)
    ATOM_SET_THE_OFFSET_(122, 890)
    ATOM_SET_THE_OFFSET_(123, 891)
    ATOM_SET_THE_OFFSET_(124, 892)
    ATOM_SET_THE_OFFSET_(125, 893)
    ATOM_SET_THE_OFFSET_(126, 894)
    ATOM_SET_THE_OFFSET_(127, 895)
    ATOM_SET_THE_OFFSET_(128, 896)
    ATOM_SET_THE_OFFSET_(129, 897)
    ATOM_SET_THE_OFFSET_(130, 898)
    ATOM_SET_THE_OFFSET_(131, 899)
    ATOM_SET_THE_OFFSET_(132, 900)
    ATOM_SET_THE_OFFSET_(133, 901)
    ATOM_SET_THE_OFFSET_(134, 902)
    ATOM_SET_THE_OFFSET_(135, 903)
    ATOM_SET_THE_OFFSET_(136, 904)
    ATOM_SET_THE_OFFSET_(137, 905)
    ATOM_SET_THE_OFFSET_(138, 906)
    ATOM_SET_THE_OFFSET_(139, 907)
    ATOM_SET_THE_OFFSET_(140, 908)
    ATOM_SET_THE_OFFSET_(141, 909)
    ATOM_SET_THE_OFFSET_(142, 910)
    ATOM_SET_THE_OFFSET_(143, 911)
    ATOM_SET_THE_OFFSET_(144, 912)
    ATOM_SET_THE_OFFSET_(145, 913)
    ATOM_SET_THE_OFFSET_(146, 914)
    ATOM_SET_THE_OFFSET_(147, 915)
    ATOM_SET_THE_OFFSET_(148, 916)
    ATOM_SET_THE_OFFSET_(149, 917)
    ATOM_SET_THE_OFFSET_(150, 918)
    ATOM_SET_THE_OFFSET_(151, 919)
    ATOM_SET_THE_OFFSET_(152, 920)
    ATOM_SET_THE_OFFSET_(153, 921)
    ATOM_SET_THE_OFFSET_(154, 922)
    ATOM_SET_THE_OFFSET_(155, 923)
    ATOM_SET_THE_OFFSET_(156, 924)
    ATOM_SET_THE_OFFSET_(157, 925)
    ATOM_SET_THE_OFFSET_(158, 926)
    ATOM_SET_THE_OFFSET_(159, 927)
    ATOM_SET_THE_OFFSET_(160, 928)
    ATOM_SET_THE_OFFSET_(161, 929)
    ATOM_SET_THE_OFFSET_(162, 930)
    ATOM_SET_THE_OFFSET_(163, 931)
    ATOM_SET_THE_OFFSET_(164, 932)
    ATOM_SET_THE_OFFSET_(165, 933)
    ATOM_SET_THE_OFFSET_(166, 934)
    ATOM_SET_THE_OFFSET_(167, 935)
    ATOM_SET_THE_OFFSET_(168, 936)
    ATOM_SET_THE_OFFSET_(169, 937)
    ATOM_SET_THE_OFFSET_(170, 938)
    ATOM_SET_THE_OFFSET_(171, 939)
    ATOM_SET_THE_OFFSET_(172, 940)
    ATOM_SET_THE_OFFSET_(173, 941)
    ATOM_SET_THE_OFFSET_(174, 942)
    ATOM_SET_THE_OFFSET_(175, 943)
    ATOM_SET_THE_OFFSET_(176, 944)
    ATOM_SET_THE_OFFSET_(177, 945)
    ATOM_SET_THE_OFFSET_(178, 946)
    ATOM_SET_THE_OFFSET_(179, 947)
    ATOM_SET_THE_OFFSET_(180, 948)
    ATOM_SET_THE_OFFSET_(181, 949)
    ATOM_SET_THE_OFFSET_(182, 950)
    ATOM_SET_THE_OFFSET_(183, 951)
    ATOM_SET_THE_OFFSET_(184, 952)
    ATOM_SET_THE_OFFSET_(185, 953)
    ATOM_SET_THE_OFFSET_(186, 954)
    ATOM_SET_THE_OFFSET_(187, 955)
    ATOM_SET_THE_OFFSET_(188, 956)
    ATOM_SET_THE_OFFSET_(189, 957)
    ATOM_SET_THE_OFFSET_(190, 958)
    ATOM_SET_THE_OFFSET_(191, 959)
    ATOM_SET_THE_OFFSET_(192, 960)
    ATOM_SET_THE_OFFSET_(193, 961)
    ATOM_SET_THE_OFFSET_(194, 962)
    ATOM_SET_THE_OFFSET_(195, 963)
    ATOM_SET_THE_OFFSET_(196, 964)
    ATOM_SET_THE_OFFSET_(197, 965)
    ATOM_SET_THE_OFFSET_(198, 966)
    ATOM_SET_THE_OFFSET_(199, 967)
    ATOM_SET_THE_OFFSET_(200, 968)
    ATOM_SET_THE_OFFSET_(201, 969)
    ATOM_SET_THE_OFFSET_(202, 970)
    ATOM_SET_THE_OFFSET_(203, 971)
    ATOM_SET_THE_OFFSET_(204, 972)
    ATOM_SET_THE_OFFSET_(205, 973)
    ATOM_SET_THE_OFFSET_(206, 974)
    ATOM_SET_THE_OFFSET_(207, 975)
    ATOM_SET_THE_OFFSET_(208, 976)
    ATOM_SET_THE_OFFSET_(209, 977)
    ATOM_SET_THE_OFFSET_(210, 978)
    ATOM_SET_THE_OFFSET_(211, 979)
    ATOM_SET_THE_OFFSET_(212, 980)
    ATOM_SET_THE_OFFSET_(213, 981)
    ATOM_SET_THE_OFFSET_(214, 982)
    ATOM_SET_THE_OFFSET_(215, 983)
    ATOM_SET_THE_OFFSET_(216, 984)
    ATOM_SET_THE_OFFSET_(217, 985)
    ATOM_SET_THE_OFFSET_(218, 986)
    ATOM_SET_THE_OFFSET_(219, 987)
    ATOM_SET_THE_OFFSET_(220, 988)
    ATOM_SET_THE_OFFSET_(221, 989)
    ATOM_SET_THE_OFFSET_(222, 990)
    ATOM_SET_THE_OFFSET_(223, 991)
    ATOM_SET_THE_OFFSET_(224, 992)
    ATOM_SET_THE_OFFSET_(225, 993)
    ATOM_SET_THE_OFFSET_(226, 994)
    ATOM_SET_THE_OFFSET_(227, 995)
    ATOM_SET_THE_OFFSET_(228, 996)
    ATOM_SET_THE_OFFSET_(229, 997)
    ATOM_SET_THE_OFFSET_(230, 998)
    ATOM_SET_THE_OFFSET_(231, 999)
    ATOM_SET_THE_OFFSET_(232, 1000)
    ATOM_SET_THE_OFFSET_(233, 1001)
    ATOM_SET_THE_OFFSET_(234, 1002)
    ATOM_SET_THE_OFFSET_(235, 1003)
    ATOM_SET_THE_OFFSET_(236, 1004)
    ATOM_SET_THE_OFFSET_(237, 1005)
    ATOM_SET_THE_OFFSET_(238, 1006)
    ATOM_SET_THE_OFFSET_(239, 1007)
    ATOM_SET_THE_OFFSET_(240, 1008)
    ATOM_SET_THE_OFFSET_(241, 1009)
    ATOM_SET_THE_OFFSET_(242, 1010)
    ATOM_SET_THE_OFFSET_(243, 1011)
    ATOM_SET_THE_OFFSET_(244, 1012)
    ATOM_SET_THE_OFFSET_(245, 1013)
    ATOM_SET_THE_OFFSET_(246, 1014)
    ATOM_SET_THE_OFFSET_(247, 1015)
    ATOM_SET_THE_OFFSET_(248, 1016)
    ATOM_SET_THE_OFFSET_(249, 1017)
    ATOM_SET_THE_OFFSET_(250, 1018)
    ATOM_SET_THE_OFFSET_(251, 1019)
    ATOM_SET_THE_OFFSET_(252, 1020)
    ATOM_SET_THE_OFFSET_(253, 1021)
    ATOM_SET_THE_OFFSET_(254, 1022)
    ATOM_SET_THE_OFFSET_(255, 1023)
    return arr;
}

consteval static auto offset_map4(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 1024)
    ATOM_SET_THE_OFFSET_(1, 1025)
    ATOM_SET_THE_OFFSET_(2, 1026)
    ATOM_SET_THE_OFFSET_(3, 1027)
    ATOM_SET_THE_OFFSET_(4, 1028)
    ATOM_SET_THE_OFFSET_(5, 1029)
    ATOM_SET_THE_OFFSET_(6, 1030)
    ATOM_SET_THE_OFFSET_(7, 1031)
    ATOM_SET_THE_OFFSET_(8, 1032)
    ATOM_SET_THE_OFFSET_(9, 1033)
    ATOM_SET_THE_OFFSET_(10, 1034)
    ATOM_SET_THE_OFFSET_(11, 1035)
    ATOM_SET_THE_OFFSET_(12, 1036)
    ATOM_SET_THE_OFFSET_(13, 1037)
    ATOM_SET_THE_OFFSET_(14, 1038)
    ATOM_SET_THE_OFFSET_(15, 1039)
    ATOM_SET_THE_OFFSET_(16, 1040)
    ATOM_SET_THE_OFFSET_(17, 1041)
    ATOM_SET_THE_OFFSET_(18, 1042)
    ATOM_SET_THE_OFFSET_(19, 1043)
    ATOM_SET_THE_OFFSET_(20, 1044)
    ATOM_SET_THE_OFFSET_(21, 1045)
    ATOM_SET_THE_OFFSET_(22, 1046)
    ATOM_SET_THE_OFFSET_(23, 1047)
    ATOM_SET_THE_OFFSET_(24, 1048)
    ATOM_SET_THE_OFFSET_(25, 1049)
    ATOM_SET_THE_OFFSET_(26, 1050)
    ATOM_SET_THE_OFFSET_(27, 1051)
    ATOM_SET_THE_OFFSET_(28, 1052)
    ATOM_SET_THE_OFFSET_(29, 1053)
    ATOM_SET_THE_OFFSET_(30, 1054)
    ATOM_SET_THE_OFFSET_(31, 1055)
    ATOM_SET_THE_OFFSET_(32, 1056)
    ATOM_SET_THE_OFFSET_(33, 1057)
    ATOM_SET_THE_OFFSET_(34, 1058)
    ATOM_SET_THE_OFFSET_(35, 1059)
    ATOM_SET_THE_OFFSET_(36, 1060)
    ATOM_SET_THE_OFFSET_(37, 1061)
    ATOM_SET_THE_OFFSET_(38, 1062)
    ATOM_SET_THE_OFFSET_(39, 1063)
    ATOM_SET_THE_OFFSET_(40, 1064)
    ATOM_SET_THE_OFFSET_(41, 1065)
    ATOM_SET_THE_OFFSET_(42, 1066)
    ATOM_SET_THE_OFFSET_(43, 1067)
    ATOM_SET_THE_OFFSET_(44, 1068)
    ATOM_SET_THE_OFFSET_(45, 1069)
    ATOM_SET_THE_OFFSET_(46, 1070)
    ATOM_SET_THE_OFFSET_(47, 1071)
    ATOM_SET_THE_OFFSET_(48, 1072)
    ATOM_SET_THE_OFFSET_(49, 1073)
    ATOM_SET_THE_OFFSET_(50, 1074)
    ATOM_SET_THE_OFFSET_(51, 1075)
    ATOM_SET_THE_OFFSET_(52, 1076)
    ATOM_SET_THE_OFFSET_(53, 1077)
    ATOM_SET_THE_OFFSET_(54, 1078)
    ATOM_SET_THE_OFFSET_(55, 1079)
    ATOM_SET_THE_OFFSET_(56, 1080)
    ATOM_SET_THE_OFFSET_(57, 1081)
    ATOM_SET_THE_OFFSET_(58, 1082)
    ATOM_SET_THE_OFFSET_(59, 1083)
    ATOM_SET_THE_OFFSET_(60, 1084)
    ATOM_SET_THE_OFFSET_(61, 1085)
    ATOM_SET_THE_OFFSET_(62, 1086)
    ATOM_SET_THE_OFFSET_(63, 1087)
    ATOM_SET_THE_OFFSET_(64, 1088)
    ATOM_SET_THE_OFFSET_(65, 1089)
    ATOM_SET_THE_OFFSET_(66, 1090)
    ATOM_SET_THE_OFFSET_(67, 1091)
    ATOM_SET_THE_OFFSET_(68, 1092)
    ATOM_SET_THE_OFFSET_(69, 1093)
    ATOM_SET_THE_OFFSET_(70, 1094)
    ATOM_SET_THE_OFFSET_(71, 1095)
    ATOM_SET_THE_OFFSET_(72, 1096)
    ATOM_SET_THE_OFFSET_(73, 1097)
    ATOM_SET_THE_OFFSET_(74, 1098)
    ATOM_SET_THE_OFFSET_(75, 1099)
    ATOM_SET_THE_OFFSET_(76, 1100)
    ATOM_SET_THE_OFFSET_(77, 1101)
    ATOM_SET_THE_OFFSET_(78, 1102)
    ATOM_SET_THE_OFFSET_(79, 1103)
    ATOM_SET_THE_OFFSET_(80, 1104)
    ATOM_SET_THE_OFFSET_(81, 1105)
    ATOM_SET_THE_OFFSET_(82, 1106)
    ATOM_SET_THE_OFFSET_(83, 1107)
    ATOM_SET_THE_OFFSET_(84, 1108)
    ATOM_SET_THE_OFFSET_(85, 1109)
    ATOM_SET_THE_OFFSET_(86, 1110)
    ATOM_SET_THE_OFFSET_(87, 1111)
    ATOM_SET_THE_OFFSET_(88, 1112)
    ATOM_SET_THE_OFFSET_(89, 1113)
    ATOM_SET_THE_OFFSET_(90, 1114)
    ATOM_SET_THE_OFFSET_(91, 1115)
    ATOM_SET_THE_OFFSET_(92, 1116)
    ATOM_SET_THE_OFFSET_(93, 1117)
    ATOM_SET_THE_OFFSET_(94, 1118)
    ATOM_SET_THE_OFFSET_(95, 1119)
    ATOM_SET_THE_OFFSET_(96, 1120)
    ATOM_SET_THE_OFFSET_(97, 1121)
    ATOM_SET_THE_OFFSET_(98, 1122)
    ATOM_SET_THE_OFFSET_(99, 1123)
    ATOM_SET_THE_OFFSET_(100, 1124)
    ATOM_SET_THE_OFFSET_(101, 1125)
    ATOM_SET_THE_OFFSET_(102, 1126)
    ATOM_SET_THE_OFFSET_(103, 1127)
    ATOM_SET_THE_OFFSET_(104, 1128)
    ATOM_SET_THE_OFFSET_(105, 1129)
    ATOM_SET_THE_OFFSET_(106, 1130)
    ATOM_SET_THE_OFFSET_(107, 1131)
    ATOM_SET_THE_OFFSET_(108, 1132)
    ATOM_SET_THE_OFFSET_(109, 1133)
    ATOM_SET_THE_OFFSET_(110, 1134)
    ATOM_SET_THE_OFFSET_(111, 1135)
    ATOM_SET_THE_OFFSET_(112, 1136)
    ATOM_SET_THE_OFFSET_(113, 1137)
    ATOM_SET_THE_OFFSET_(114, 1138)
    ATOM_SET_THE_OFFSET_(115, 1139)
    ATOM_SET_THE_OFFSET_(116, 1140)
    ATOM_SET_THE_OFFSET_(117, 1141)
    ATOM_SET_THE_OFFSET_(118, 1142)
    ATOM_SET_THE_OFFSET_(119, 1143)
    ATOM_SET_THE_OFFSET_(120, 1144)
    ATOM_SET_THE_OFFSET_(121, 1145)
    ATOM_SET_THE_OFFSET_(122, 1146)
    ATOM_SET_THE_OFFSET_(123, 1147)
    ATOM_SET_THE_OFFSET_(124, 1148)
    ATOM_SET_THE_OFFSET_(125, 1149)
    ATOM_SET_THE_OFFSET_(126, 1150)
    ATOM_SET_THE_OFFSET_(127, 1151)
    ATOM_SET_THE_OFFSET_(128, 1152)
    ATOM_SET_THE_OFFSET_(129, 1153)
    ATOM_SET_THE_OFFSET_(130, 1154)
    ATOM_SET_THE_OFFSET_(131, 1155)
    ATOM_SET_THE_OFFSET_(132, 1156)
    ATOM_SET_THE_OFFSET_(133, 1157)
    ATOM_SET_THE_OFFSET_(134, 1158)
    ATOM_SET_THE_OFFSET_(135, 1159)
    ATOM_SET_THE_OFFSET_(136, 1160)
    ATOM_SET_THE_OFFSET_(137, 1161)
    ATOM_SET_THE_OFFSET_(138, 1162)
    ATOM_SET_THE_OFFSET_(139, 1163)
    ATOM_SET_THE_OFFSET_(140, 1164)
    ATOM_SET_THE_OFFSET_(141, 1165)
    ATOM_SET_THE_OFFSET_(142, 1166)
    ATOM_SET_THE_OFFSET_(143, 1167)
    ATOM_SET_THE_OFFSET_(144, 1168)
    ATOM_SET_THE_OFFSET_(145, 1169)
    ATOM_SET_THE_OFFSET_(146, 1170)
    ATOM_SET_THE_OFFSET_(147, 1171)
    ATOM_SET_THE_OFFSET_(148, 1172)
    ATOM_SET_THE_OFFSET_(149, 1173)
    ATOM_SET_THE_OFFSET_(150, 1174)
    ATOM_SET_THE_OFFSET_(151, 1175)
    ATOM_SET_THE_OFFSET_(152, 1176)
    ATOM_SET_THE_OFFSET_(153, 1177)
    ATOM_SET_THE_OFFSET_(154, 1178)
    ATOM_SET_THE_OFFSET_(155, 1179)
    ATOM_SET_THE_OFFSET_(156, 1180)
    ATOM_SET_THE_OFFSET_(157, 1181)
    ATOM_SET_THE_OFFSET_(158, 1182)
    ATOM_SET_THE_OFFSET_(159, 1183)
    ATOM_SET_THE_OFFSET_(160, 1184)
    ATOM_SET_THE_OFFSET_(161, 1185)
    ATOM_SET_THE_OFFSET_(162, 1186)
    ATOM_SET_THE_OFFSET_(163, 1187)
    ATOM_SET_THE_OFFSET_(164, 1188)
    ATOM_SET_THE_OFFSET_(165, 1189)
    ATOM_SET_THE_OFFSET_(166, 1190)
    ATOM_SET_THE_OFFSET_(167, 1191)
    ATOM_SET_THE_OFFSET_(168, 1192)
    ATOM_SET_THE_OFFSET_(169, 1193)
    ATOM_SET_THE_OFFSET_(170, 1194)
    ATOM_SET_THE_OFFSET_(171, 1195)
    ATOM_SET_THE_OFFSET_(172, 1196)
    ATOM_SET_THE_OFFSET_(173, 1197)
    ATOM_SET_THE_OFFSET_(174, 1198)
    ATOM_SET_THE_OFFSET_(175, 1199)
    ATOM_SET_THE_OFFSET_(176, 1200)
    ATOM_SET_THE_OFFSET_(177, 1201)
    ATOM_SET_THE_OFFSET_(178, 1202)
    ATOM_SET_THE_OFFSET_(179, 1203)
    ATOM_SET_THE_OFFSET_(180, 1204)
    ATOM_SET_THE_OFFSET_(181, 1205)
    ATOM_SET_THE_OFFSET_(182, 1206)
    ATOM_SET_THE_OFFSET_(183, 1207)
    ATOM_SET_THE_OFFSET_(184, 1208)
    ATOM_SET_THE_OFFSET_(185, 1209)
    ATOM_SET_THE_OFFSET_(186, 1210)
    ATOM_SET_THE_OFFSET_(187, 1211)
    ATOM_SET_THE_OFFSET_(188, 1212)
    ATOM_SET_THE_OFFSET_(189, 1213)
    ATOM_SET_THE_OFFSET_(190, 1214)
    ATOM_SET_THE_OFFSET_(191, 1215)
    ATOM_SET_THE_OFFSET_(192, 1216)
    ATOM_SET_THE_OFFSET_(193, 1217)
    ATOM_SET_THE_OFFSET_(194, 1218)
    ATOM_SET_THE_OFFSET_(195, 1219)
    ATOM_SET_THE_OFFSET_(196, 1220)
    ATOM_SET_THE_OFFSET_(197, 1221)
    ATOM_SET_THE_OFFSET_(198, 1222)
    ATOM_SET_THE_OFFSET_(199, 1223)
    ATOM_SET_THE_OFFSET_(200, 1224)
    ATOM_SET_THE_OFFSET_(201, 1225)
    ATOM_SET_THE_OFFSET_(202, 1226)
    ATOM_SET_THE_OFFSET_(203, 1227)
    ATOM_SET_THE_OFFSET_(204, 1228)
    ATOM_SET_THE_OFFSET_(205, 1229)
    ATOM_SET_THE_OFFSET_(206, 1230)
    ATOM_SET_THE_OFFSET_(207, 1231)
    ATOM_SET_THE_OFFSET_(208, 1232)
    ATOM_SET_THE_OFFSET_(209, 1233)
    ATOM_SET_THE_OFFSET_(210, 1234)
    ATOM_SET_THE_OFFSET_(211, 1235)
    ATOM_SET_THE_OFFSET_(212, 1236)
    ATOM_SET_THE_OFFSET_(213, 1237)
    ATOM_SET_THE_OFFSET_(214, 1238)
    ATOM_SET_THE_OFFSET_(215, 1239)
    ATOM_SET_THE_OFFSET_(216, 1240)
    ATOM_SET_THE_OFFSET_(217, 1241)
    ATOM_SET_THE_OFFSET_(218, 1242)
    ATOM_SET_THE_OFFSET_(219, 1243)
    ATOM_SET_THE_OFFSET_(220, 1244)
    ATOM_SET_THE_OFFSET_(221, 1245)
    ATOM_SET_THE_OFFSET_(222, 1246)
    ATOM_SET_THE_OFFSET_(223, 1247)
    ATOM_SET_THE_OFFSET_(224, 1248)
    ATOM_SET_THE_OFFSET_(225, 1249)
    ATOM_SET_THE_OFFSET_(226, 1250)
    ATOM_SET_THE_OFFSET_(227, 1251)
    ATOM_SET_THE_OFFSET_(228, 1252)
    ATOM_SET_THE_OFFSET_(229, 1253)
    ATOM_SET_THE_OFFSET_(230, 1254)
    ATOM_SET_THE_OFFSET_(231, 1255)
    ATOM_SET_THE_OFFSET_(232, 1256)
    ATOM_SET_THE_OFFSET_(233, 1257)
    ATOM_SET_THE_OFFSET_(234, 1258)
    ATOM_SET_THE_OFFSET_(235, 1259)
    ATOM_SET_THE_OFFSET_(236, 1260)
    ATOM_SET_THE_OFFSET_(237, 1261)
    ATOM_SET_THE_OFFSET_(238, 1262)
    ATOM_SET_THE_OFFSET_(239, 1263)
    ATOM_SET_THE_OFFSET_(240, 1264)
    ATOM_SET_THE_OFFSET_(241, 1265)
    ATOM_SET_THE_OFFSET_(242, 1266)
    ATOM_SET_THE_OFFSET_(243, 1267)
    ATOM_SET_THE_OFFSET_(244, 1268)
    ATOM_SET_THE_OFFSET_(245, 1269)
    ATOM_SET_THE_OFFSET_(246, 1270)
    ATOM_SET_THE_OFFSET_(247, 1271)
    ATOM_SET_THE_OFFSET_(248, 1272)
    ATOM_SET_THE_OFFSET_(249, 1273)
    ATOM_SET_THE_OFFSET_(250, 1274)
    ATOM_SET_THE_OFFSET_(251, 1275)
    ATOM_SET_THE_OFFSET_(252, 1276)
    ATOM_SET_THE_OFFSET_(253, 1277)
    ATOM_SET_THE_OFFSET_(254, 1278)
    ATOM_SET_THE_OFFSET_(255, 1279)
    return arr;
}

consteval static auto offset_map5(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 1280)
    ATOM_SET_THE_OFFSET_(1, 1281)
    ATOM_SET_THE_OFFSET_(2, 1282)
    ATOM_SET_THE_OFFSET_(3, 1283)
    ATOM_SET_THE_OFFSET_(4, 1284)
    ATOM_SET_THE_OFFSET_(5, 1285)
    ATOM_SET_THE_OFFSET_(6, 1286)
    ATOM_SET_THE_OFFSET_(7, 1287)
    ATOM_SET_THE_OFFSET_(8, 1288)
    ATOM_SET_THE_OFFSET_(9, 1289)
    ATOM_SET_THE_OFFSET_(10, 1290)
    ATOM_SET_THE_OFFSET_(11, 1291)
    ATOM_SET_THE_OFFSET_(12, 1292)
    ATOM_SET_THE_OFFSET_(13, 1293)
    ATOM_SET_THE_OFFSET_(14, 1294)
    ATOM_SET_THE_OFFSET_(15, 1295)
    ATOM_SET_THE_OFFSET_(16, 1296)
    ATOM_SET_THE_OFFSET_(17, 1297)
    ATOM_SET_THE_OFFSET_(18, 1298)
    ATOM_SET_THE_OFFSET_(19, 1299)
    ATOM_SET_THE_OFFSET_(20, 1300)
    ATOM_SET_THE_OFFSET_(21, 1301)
    ATOM_SET_THE_OFFSET_(22, 1302)
    ATOM_SET_THE_OFFSET_(23, 1303)
    ATOM_SET_THE_OFFSET_(24, 1304)
    ATOM_SET_THE_OFFSET_(25, 1305)
    ATOM_SET_THE_OFFSET_(26, 1306)
    ATOM_SET_THE_OFFSET_(27, 1307)
    ATOM_SET_THE_OFFSET_(28, 1308)
    ATOM_SET_THE_OFFSET_(29, 1309)
    ATOM_SET_THE_OFFSET_(30, 1310)
    ATOM_SET_THE_OFFSET_(31, 1311)
    ATOM_SET_THE_OFFSET_(32, 1312)
    ATOM_SET_THE_OFFSET_(33, 1313)
    ATOM_SET_THE_OFFSET_(34, 1314)
    ATOM_SET_THE_OFFSET_(35, 1315)
    ATOM_SET_THE_OFFSET_(36, 1316)
    ATOM_SET_THE_OFFSET_(37, 1317)
    ATOM_SET_THE_OFFSET_(38, 1318)
    ATOM_SET_THE_OFFSET_(39, 1319)
    ATOM_SET_THE_OFFSET_(40, 1320)
    ATOM_SET_THE_OFFSET_(41, 1321)
    ATOM_SET_THE_OFFSET_(42, 1322)
    ATOM_SET_THE_OFFSET_(43, 1323)
    ATOM_SET_THE_OFFSET_(44, 1324)
    ATOM_SET_THE_OFFSET_(45, 1325)
    ATOM_SET_THE_OFFSET_(46, 1326)
    ATOM_SET_THE_OFFSET_(47, 1327)
    ATOM_SET_THE_OFFSET_(48, 1328)
    ATOM_SET_THE_OFFSET_(49, 1329)
    ATOM_SET_THE_OFFSET_(50, 1330)
    ATOM_SET_THE_OFFSET_(51, 1331)
    ATOM_SET_THE_OFFSET_(52, 1332)
    ATOM_SET_THE_OFFSET_(53, 1333)
    ATOM_SET_THE_OFFSET_(54, 1334)
    ATOM_SET_THE_OFFSET_(55, 1335)
    ATOM_SET_THE_OFFSET_(56, 1336)
    ATOM_SET_THE_OFFSET_(57, 1337)
    ATOM_SET_THE_OFFSET_(58, 1338)
    ATOM_SET_THE_OFFSET_(59, 1339)
    ATOM_SET_THE_OFFSET_(60, 1340)
    ATOM_SET_THE_OFFSET_(61, 1341)
    ATOM_SET_THE_OFFSET_(62, 1342)
    ATOM_SET_THE_OFFSET_(63, 1343)
    ATOM_SET_THE_OFFSET_(64, 1344)
    ATOM_SET_THE_OFFSET_(65, 1345)
    ATOM_SET_THE_OFFSET_(66, 1346)
    ATOM_SET_THE_OFFSET_(67, 1347)
    ATOM_SET_THE_OFFSET_(68, 1348)
    ATOM_SET_THE_OFFSET_(69, 1349)
    ATOM_SET_THE_OFFSET_(70, 1350)
    ATOM_SET_THE_OFFSET_(71, 1351)
    ATOM_SET_THE_OFFSET_(72, 1352)
    ATOM_SET_THE_OFFSET_(73, 1353)
    ATOM_SET_THE_OFFSET_(74, 1354)
    ATOM_SET_THE_OFFSET_(75, 1355)
    ATOM_SET_THE_OFFSET_(76, 1356)
    ATOM_SET_THE_OFFSET_(77, 1357)
    ATOM_SET_THE_OFFSET_(78, 1358)
    ATOM_SET_THE_OFFSET_(79, 1359)
    ATOM_SET_THE_OFFSET_(80, 1360)
    ATOM_SET_THE_OFFSET_(81, 1361)
    ATOM_SET_THE_OFFSET_(82, 1362)
    ATOM_SET_THE_OFFSET_(83, 1363)
    ATOM_SET_THE_OFFSET_(84, 1364)
    ATOM_SET_THE_OFFSET_(85, 1365)
    ATOM_SET_THE_OFFSET_(86, 1366)
    ATOM_SET_THE_OFFSET_(87, 1367)
    ATOM_SET_THE_OFFSET_(88, 1368)
    ATOM_SET_THE_OFFSET_(89, 1369)
    ATOM_SET_THE_OFFSET_(90, 1370)
    ATOM_SET_THE_OFFSET_(91, 1371)
    ATOM_SET_THE_OFFSET_(92, 1372)
    ATOM_SET_THE_OFFSET_(93, 1373)
    ATOM_SET_THE_OFFSET_(94, 1374)
    ATOM_SET_THE_OFFSET_(95, 1375)
    ATOM_SET_THE_OFFSET_(96, 1376)
    ATOM_SET_THE_OFFSET_(97, 1377)
    ATOM_SET_THE_OFFSET_(98, 1378)
    ATOM_SET_THE_OFFSET_(99, 1379)
    ATOM_SET_THE_OFFSET_(100, 1380)
    ATOM_SET_THE_OFFSET_(101, 1381)
    ATOM_SET_THE_OFFSET_(102, 1382)
    ATOM_SET_THE_OFFSET_(103, 1383)
    ATOM_SET_THE_OFFSET_(104, 1384)
    ATOM_SET_THE_OFFSET_(105, 1385)
    ATOM_SET_THE_OFFSET_(106, 1386)
    ATOM_SET_THE_OFFSET_(107, 1387)
    ATOM_SET_THE_OFFSET_(108, 1388)
    ATOM_SET_THE_OFFSET_(109, 1389)
    ATOM_SET_THE_OFFSET_(110, 1390)
    ATOM_SET_THE_OFFSET_(111, 1391)
    ATOM_SET_THE_OFFSET_(112, 1392)
    ATOM_SET_THE_OFFSET_(113, 1393)
    ATOM_SET_THE_OFFSET_(114, 1394)
    ATOM_SET_THE_OFFSET_(115, 1395)
    ATOM_SET_THE_OFFSET_(116, 1396)
    ATOM_SET_THE_OFFSET_(117, 1397)
    ATOM_SET_THE_OFFSET_(118, 1398)
    ATOM_SET_THE_OFFSET_(119, 1399)
    ATOM_SET_THE_OFFSET_(120, 1400)
    ATOM_SET_THE_OFFSET_(121, 1401)
    ATOM_SET_THE_OFFSET_(122, 1402)
    ATOM_SET_THE_OFFSET_(123, 1403)
    ATOM_SET_THE_OFFSET_(124, 1404)
    ATOM_SET_THE_OFFSET_(125, 1405)
    ATOM_SET_THE_OFFSET_(126, 1406)
    ATOM_SET_THE_OFFSET_(127, 1407)
    ATOM_SET_THE_OFFSET_(128, 1408)
    ATOM_SET_THE_OFFSET_(129, 1409)
    ATOM_SET_THE_OFFSET_(130, 1410)
    ATOM_SET_THE_OFFSET_(131, 1411)
    ATOM_SET_THE_OFFSET_(132, 1412)
    ATOM_SET_THE_OFFSET_(133, 1413)
    ATOM_SET_THE_OFFSET_(134, 1414)
    ATOM_SET_THE_OFFSET_(135, 1415)
    ATOM_SET_THE_OFFSET_(136, 1416)
    ATOM_SET_THE_OFFSET_(137, 1417)
    ATOM_SET_THE_OFFSET_(138, 1418)
    ATOM_SET_THE_OFFSET_(139, 1419)
    ATOM_SET_THE_OFFSET_(140, 1420)
    ATOM_SET_THE_OFFSET_(141, 1421)
    ATOM_SET_THE_OFFSET_(142, 1422)
    ATOM_SET_THE_OFFSET_(143, 1423)
    ATOM_SET_THE_OFFSET_(144, 1424)
    ATOM_SET_THE_OFFSET_(145, 1425)
    ATOM_SET_THE_OFFSET_(146, 1426)
    ATOM_SET_THE_OFFSET_(147, 1427)
    ATOM_SET_THE_OFFSET_(148, 1428)
    ATOM_SET_THE_OFFSET_(149, 1429)
    ATOM_SET_THE_OFFSET_(150, 1430)
    ATOM_SET_THE_OFFSET_(151, 1431)
    ATOM_SET_THE_OFFSET_(152, 1432)
    ATOM_SET_THE_OFFSET_(153, 1433)
    ATOM_SET_THE_OFFSET_(154, 1434)
    ATOM_SET_THE_OFFSET_(155, 1435)
    ATOM_SET_THE_OFFSET_(156, 1436)
    ATOM_SET_THE_OFFSET_(157, 1437)
    ATOM_SET_THE_OFFSET_(158, 1438)
    ATOM_SET_THE_OFFSET_(159, 1439)
    ATOM_SET_THE_OFFSET_(160, 1440)
    ATOM_SET_THE_OFFSET_(161, 1441)
    ATOM_SET_THE_OFFSET_(162, 1442)
    ATOM_SET_THE_OFFSET_(163, 1443)
    ATOM_SET_THE_OFFSET_(164, 1444)
    ATOM_SET_THE_OFFSET_(165, 1445)
    ATOM_SET_THE_OFFSET_(166, 1446)
    ATOM_SET_THE_OFFSET_(167, 1447)
    ATOM_SET_THE_OFFSET_(168, 1448)
    ATOM_SET_THE_OFFSET_(169, 1449)
    ATOM_SET_THE_OFFSET_(170, 1450)
    ATOM_SET_THE_OFFSET_(171, 1451)
    ATOM_SET_THE_OFFSET_(172, 1452)
    ATOM_SET_THE_OFFSET_(173, 1453)
    ATOM_SET_THE_OFFSET_(174, 1454)
    ATOM_SET_THE_OFFSET_(175, 1455)
    ATOM_SET_THE_OFFSET_(176, 1456)
    ATOM_SET_THE_OFFSET_(177, 1457)
    ATOM_SET_THE_OFFSET_(178, 1458)
    ATOM_SET_THE_OFFSET_(179, 1459)
    ATOM_SET_THE_OFFSET_(180, 1460)
    ATOM_SET_THE_OFFSET_(181, 1461)
    ATOM_SET_THE_OFFSET_(182, 1462)
    ATOM_SET_THE_OFFSET_(183, 1463)
    ATOM_SET_THE_OFFSET_(184, 1464)
    ATOM_SET_THE_OFFSET_(185, 1465)
    ATOM_SET_THE_OFFSET_(186, 1466)
    ATOM_SET_THE_OFFSET_(187, 1467)
    ATOM_SET_THE_OFFSET_(188, 1468)
    ATOM_SET_THE_OFFSET_(189, 1469)
    ATOM_SET_THE_OFFSET_(190, 1470)
    ATOM_SET_THE_OFFSET_(191, 1471)
    ATOM_SET_THE_OFFSET_(192, 1472)
    ATOM_SET_THE_OFFSET_(193, 1473)
    ATOM_SET_THE_OFFSET_(194, 1474)
    ATOM_SET_THE_OFFSET_(195, 1475)
    ATOM_SET_THE_OFFSET_(196, 1476)
    ATOM_SET_THE_OFFSET_(197, 1477)
    ATOM_SET_THE_OFFSET_(198, 1478)
    ATOM_SET_THE_OFFSET_(199, 1479)
    ATOM_SET_THE_OFFSET_(200, 1480)
    ATOM_SET_THE_OFFSET_(201, 1481)
    ATOM_SET_THE_OFFSET_(202, 1482)
    ATOM_SET_THE_OFFSET_(203, 1483)
    ATOM_SET_THE_OFFSET_(204, 1484)
    ATOM_SET_THE_OFFSET_(205, 1485)
    ATOM_SET_THE_OFFSET_(206, 1486)
    ATOM_SET_THE_OFFSET_(207, 1487)
    ATOM_SET_THE_OFFSET_(208, 1488)
    ATOM_SET_THE_OFFSET_(209, 1489)
    ATOM_SET_THE_OFFSET_(210, 1490)
    ATOM_SET_THE_OFFSET_(211, 1491)
    ATOM_SET_THE_OFFSET_(212, 1492)
    ATOM_SET_THE_OFFSET_(213, 1493)
    ATOM_SET_THE_OFFSET_(214, 1494)
    ATOM_SET_THE_OFFSET_(215, 1495)
    ATOM_SET_THE_OFFSET_(216, 1496)
    ATOM_SET_THE_OFFSET_(217, 1497)
    ATOM_SET_THE_OFFSET_(218, 1498)
    ATOM_SET_THE_OFFSET_(219, 1499)
    ATOM_SET_THE_OFFSET_(220, 1500)
    ATOM_SET_THE_OFFSET_(221, 1501)
    ATOM_SET_THE_OFFSET_(222, 1502)
    ATOM_SET_THE_OFFSET_(223, 1503)
    ATOM_SET_THE_OFFSET_(224, 1504)
    ATOM_SET_THE_OFFSET_(225, 1505)
    ATOM_SET_THE_OFFSET_(226, 1506)
    ATOM_SET_THE_OFFSET_(227, 1507)
    ATOM_SET_THE_OFFSET_(228, 1508)
    ATOM_SET_THE_OFFSET_(229, 1509)
    ATOM_SET_THE_OFFSET_(230, 1510)
    ATOM_SET_THE_OFFSET_(231, 1511)
    ATOM_SET_THE_OFFSET_(232, 1512)
    ATOM_SET_THE_OFFSET_(233, 1513)
    ATOM_SET_THE_OFFSET_(234, 1514)
    ATOM_SET_THE_OFFSET_(235, 1515)
    ATOM_SET_THE_OFFSET_(236, 1516)
    ATOM_SET_THE_OFFSET_(237, 1517)
    ATOM_SET_THE_OFFSET_(238, 1518)
    ATOM_SET_THE_OFFSET_(239, 1519)
    ATOM_SET_THE_OFFSET_(240, 1520)
    ATOM_SET_THE_OFFSET_(241, 1521)
    ATOM_SET_THE_OFFSET_(242, 1522)
    ATOM_SET_THE_OFFSET_(243, 1523)
    ATOM_SET_THE_OFFSET_(244, 1524)
    ATOM_SET_THE_OFFSET_(245, 1525)
    ATOM_SET_THE_OFFSET_(246, 1526)
    ATOM_SET_THE_OFFSET_(247, 1527)
    ATOM_SET_THE_OFFSET_(248, 1528)
    ATOM_SET_THE_OFFSET_(249, 1529)
    ATOM_SET_THE_OFFSET_(250, 1530)
    ATOM_SET_THE_OFFSET_(251, 1531)
    ATOM_SET_THE_OFFSET_(252, 1532)
    ATOM_SET_THE_OFFSET_(253, 1533)
    ATOM_SET_THE_OFFSET_(254, 1534)
    ATOM_SET_THE_OFFSET_(255, 1535)
    return arr;
}
consteval static auto offset_map6(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 1536)
    ATOM_SET_THE_OFFSET_(1, 1537)
    ATOM_SET_THE_OFFSET_(2, 1538)
    ATOM_SET_THE_OFFSET_(3, 1539)

    ATOM_SET_THE_OFFSET_(4, 1540)
    ATOM_SET_THE_OFFSET_(5, 1541)

    ATOM_SET_THE_OFFSET_(6, 1542)
    ATOM_SET_THE_OFFSET_(7, 1543)

    ATOM_SET_THE_OFFSET_(8, 1544)
    ATOM_SET_THE_OFFSET_(9, 1545)

    ATOM_SET_THE_OFFSET_(10, 1546)
    ATOM_SET_THE_OFFSET_(11, 1547)

    ATOM_SET_THE_OFFSET_(12, 1548)
    ATOM_SET_THE_OFFSET_(13, 1549)

    ATOM_SET_THE_OFFSET_(14, 1550)
    ATOM_SET_THE_OFFSET_(15, 1551)

    ATOM_SET_THE_OFFSET_(16, 1552)
    ATOM_SET_THE_OFFSET_(17, 1553)

    ATOM_SET_THE_OFFSET_(18, 1554)
    ATOM_SET_THE_OFFSET_(19, 1555)

    ATOM_SET_THE_OFFSET_(20, 1556)
    ATOM_SET_THE_OFFSET_(21, 1557)

    ATOM_SET_THE_OFFSET_(22, 1558)
    ATOM_SET_THE_OFFSET_(23, 1559)

    ATOM_SET_THE_OFFSET_(24, 1560)
    ATOM_SET_THE_OFFSET_(25, 1561)

    ATOM_SET_THE_OFFSET_(26, 1562)
    ATOM_SET_THE_OFFSET_(27, 1563)

    ATOM_SET_THE_OFFSET_(28, 1564)
    ATOM_SET_THE_OFFSET_(29, 1565)

    ATOM_SET_THE_OFFSET_(30, 1566)
    ATOM_SET_THE_OFFSET_(31, 1567)

    ATOM_SET_THE_OFFSET_(32, 1568)
    ATOM_SET_THE_OFFSET_(33, 1569)

    ATOM_SET_THE_OFFSET_(34, 1570)
    ATOM_SET_THE_OFFSET_(35, 1571)

    ATOM_SET_THE_OFFSET_(36, 1572)
    ATOM_SET_THE_OFFSET_(37, 1573)

    ATOM_SET_THE_OFFSET_(38, 1574)
    ATOM_SET_THE_OFFSET_(39, 1575)

    ATOM_SET_THE_OFFSET_(40, 1576)
    ATOM_SET_THE_OFFSET_(41, 1577)

    ATOM_SET_THE_OFFSET_(42, 1578)
    ATOM_SET_THE_OFFSET_(43, 1579)

    ATOM_SET_THE_OFFSET_(44, 1580)
    ATOM_SET_THE_OFFSET_(45, 1581)

    ATOM_SET_THE_OFFSET_(46, 1582)
    ATOM_SET_THE_OFFSET_(47, 1583)

    ATOM_SET_THE_OFFSET_(48, 1584)
    ATOM_SET_THE_OFFSET_(49, 1585)

    ATOM_SET_THE_OFFSET_(50, 1586)
    ATOM_SET_THE_OFFSET_(51, 1587)

    ATOM_SET_THE_OFFSET_(52, 1588)
    ATOM_SET_THE_OFFSET_(53, 1589)

    ATOM_SET_THE_OFFSET_(54, 1590)
    ATOM_SET_THE_OFFSET_(55, 1591)

    ATOM_SET_THE_OFFSET_(56, 1592)
    ATOM_SET_THE_OFFSET_(57, 1593)

    ATOM_SET_THE_OFFSET_(58, 1594)
    ATOM_SET_THE_OFFSET_(59, 1595)

    ATOM_SET_THE_OFFSET_(60, 1596)
    ATOM_SET_THE_OFFSET_(61, 1597)

    ATOM_SET_THE_OFFSET_(62, 1598)
    ATOM_SET_THE_OFFSET_(63, 1599)

    ATOM_SET_THE_OFFSET_(64, 1600)
    ATOM_SET_THE_OFFSET_(65, 1601)

    ATOM_SET_THE_OFFSET_(66, 1602)
    ATOM_SET_THE_OFFSET_(67, 1603)

    ATOM_SET_THE_OFFSET_(68, 1604)
    ATOM_SET_THE_OFFSET_(69, 1605)

    ATOM_SET_THE_OFFSET_(70, 1606)
    ATOM_SET_THE_OFFSET_(71, 1607)

    ATOM_SET_THE_OFFSET_(72, 1608)
    ATOM_SET_THE_OFFSET_(73, 1609)

    ATOM_SET_THE_OFFSET_(74, 1610)
    ATOM_SET_THE_OFFSET_(75, 1611)

    ATOM_SET_THE_OFFSET_(76, 1612)
    ATOM_SET_THE_OFFSET_(77, 1613)

    ATOM_SET_THE_OFFSET_(78, 1614)
    ATOM_SET_THE_OFFSET_(79, 1615)

    ATOM_SET_THE_OFFSET_(80, 1616)
    ATOM_SET_THE_OFFSET_(81, 1617)

    ATOM_SET_THE_OFFSET_(82, 1618)
    ATOM_SET_THE_OFFSET_(83, 1619)

    ATOM_SET_THE_OFFSET_(84, 1620)
    ATOM_SET_THE_OFFSET_(85, 1621)

    ATOM_SET_THE_OFFSET_(86, 1622)
    ATOM_SET_THE_OFFSET_(87, 1623)

    ATOM_SET_THE_OFFSET_(88, 1624)
    ATOM_SET_THE_OFFSET_(89, 1625)

    ATOM_SET_THE_OFFSET_(90, 1626)
    ATOM_SET_THE_OFFSET_(91, 1627)

    ATOM_SET_THE_OFFSET_(92, 1628)
    ATOM_SET_THE_OFFSET_(93, 1629)

    ATOM_SET_THE_OFFSET_(94, 1630)
    ATOM_SET_THE_OFFSET_(95, 1631)

    ATOM_SET_THE_OFFSET_(96, 1632)
    ATOM_SET_THE_OFFSET_(97, 1633)

    ATOM_SET_THE_OFFSET_(98, 1634)
    ATOM_SET_THE_OFFSET_(99, 1635)

    ATOM_SET_THE_OFFSET_(100, 1636)
    ATOM_SET_THE_OFFSET_(101, 1637)

    ATOM_SET_THE_OFFSET_(102, 1638)
    ATOM_SET_THE_OFFSET_(103, 1639)

    ATOM_SET_THE_OFFSET_(104, 1640)
    ATOM_SET_THE_OFFSET_(105, 1641)

    ATOM_SET_THE_OFFSET_(106, 1642)
    ATOM_SET_THE_OFFSET_(107, 1643)

    ATOM_SET_THE_OFFSET_(108, 1644)
    ATOM_SET_THE_OFFSET_(109, 1645)

    ATOM_SET_THE_OFFSET_(110, 1646)
    ATOM_SET_THE_OFFSET_(111, 1647)

    ATOM_SET_THE_OFFSET_(112, 1648)
    ATOM_SET_THE_OFFSET_(113, 1649)

    ATOM_SET_THE_OFFSET_(114, 1650)
    ATOM_SET_THE_OFFSET_(115, 1651)

    ATOM_SET_THE_OFFSET_(116, 1652)
    ATOM_SET_THE_OFFSET_(117, 1653)

    ATOM_SET_THE_OFFSET_(118, 1654)
    ATOM_SET_THE_OFFSET_(119, 1655)

    ATOM_SET_THE_OFFSET_(120, 1656)
    ATOM_SET_THE_OFFSET_(121, 1657)

    ATOM_SET_THE_OFFSET_(122, 1658)
    ATOM_SET_THE_OFFSET_(123, 1659)

    ATOM_SET_THE_OFFSET_(124, 1660)
    ATOM_SET_THE_OFFSET_(125, 1661)

    ATOM_SET_THE_OFFSET_(126, 1662)
    ATOM_SET_THE_OFFSET_(127, 1663)

    ATOM_SET_THE_OFFSET_(128, 1664)
    ATOM_SET_THE_OFFSET_(129, 1665)

    ATOM_SET_THE_OFFSET_(130, 1666)
    ATOM_SET_THE_OFFSET_(131, 1667)

    ATOM_SET_THE_OFFSET_(132, 1668)
    ATOM_SET_THE_OFFSET_(133, 1669)

    ATOM_SET_THE_OFFSET_(134, 1670)
    ATOM_SET_THE_OFFSET_(135, 1671)

    ATOM_SET_THE_OFFSET_(136, 1672)
    ATOM_SET_THE_OFFSET_(137, 1673)

    ATOM_SET_THE_OFFSET_(138, 1674)
    ATOM_SET_THE_OFFSET_(139, 1675)

    ATOM_SET_THE_OFFSET_(140, 1676)
    ATOM_SET_THE_OFFSET_(141, 1677)

    ATOM_SET_THE_OFFSET_(142, 1678)
    ATOM_SET_THE_OFFSET_(143, 1679)

    ATOM_SET_THE_OFFSET_(144, 1680)
    ATOM_SET_THE_OFFSET_(145, 1681)

    ATOM_SET_THE_OFFSET_(146, 1682)
    ATOM_SET_THE_OFFSET_(147, 1683)

    ATOM_SET_THE_OFFSET_(148, 1684)
    ATOM_SET_THE_OFFSET_(149, 1685)

    ATOM_SET_THE_OFFSET_(150, 1686)
    ATOM_SET_THE_OFFSET_(151, 1687)

    ATOM_SET_THE_OFFSET_(152, 1688)
    ATOM_SET_THE_OFFSET_(153, 1689)

    ATOM_SET_THE_OFFSET_(154, 1690)
    ATOM_SET_THE_OFFSET_(155, 1691)

    ATOM_SET_THE_OFFSET_(156, 1692)
    ATOM_SET_THE_OFFSET_(157, 1693)

    ATOM_SET_THE_OFFSET_(158, 1694)
    ATOM_SET_THE_OFFSET_(159, 1695)

    ATOM_SET_THE_OFFSET_(160, 1696)
    ATOM_SET_THE_OFFSET_(161, 1697)

    ATOM_SET_THE_OFFSET_(162, 1698)
    ATOM_SET_THE_OFFSET_(163, 1699)

    ATOM_SET_THE_OFFSET_(164, 1700)
    ATOM_SET_THE_OFFSET_(165, 1701)

    ATOM_SET_THE_OFFSET_(166, 1702)
    ATOM_SET_THE_OFFSET_(167, 1703)

    ATOM_SET_THE_OFFSET_(168, 1704)
    ATOM_SET_THE_OFFSET_(169, 1705)

    ATOM_SET_THE_OFFSET_(170, 1706)
    ATOM_SET_THE_OFFSET_(171, 1707)

    ATOM_SET_THE_OFFSET_(172, 1708)
    ATOM_SET_THE_OFFSET_(173, 1709)

    ATOM_SET_THE_OFFSET_(174, 1710)
    ATOM_SET_THE_OFFSET_(175, 1711)

    ATOM_SET_THE_OFFSET_(176, 1712)
    ATOM_SET_THE_OFFSET_(177, 1713)

    ATOM_SET_THE_OFFSET_(178, 1714)
    ATOM_SET_THE_OFFSET_(179, 1715)

    ATOM_SET_THE_OFFSET_(180, 1716)
    ATOM_SET_THE_OFFSET_(181, 1717)

    ATOM_SET_THE_OFFSET_(182, 1718)
    ATOM_SET_THE_OFFSET_(183, 1719)

    ATOM_SET_THE_OFFSET_(184, 1720)
    ATOM_SET_THE_OFFSET_(185, 1721)

    ATOM_SET_THE_OFFSET_(186, 1722)
    ATOM_SET_THE_OFFSET_(187, 1723)

    ATOM_SET_THE_OFFSET_(188, 1724)
    ATOM_SET_THE_OFFSET_(189, 1725)

    ATOM_SET_THE_OFFSET_(190, 1726)
    ATOM_SET_THE_OFFSET_(191, 1727)

    ATOM_SET_THE_OFFSET_(192, 1728)
    ATOM_SET_THE_OFFSET_(193, 1729)

    ATOM_SET_THE_OFFSET_(194, 1730)
    ATOM_SET_THE_OFFSET_(195, 1731)

    ATOM_SET_THE_OFFSET_(196, 1732)
    ATOM_SET_THE_OFFSET_(197, 1733)

    ATOM_SET_THE_OFFSET_(198, 1734)
    ATOM_SET_THE_OFFSET_(199, 1735)

    ATOM_SET_THE_OFFSET_(200, 1736)
    ATOM_SET_THE_OFFSET_(201, 1737)

    ATOM_SET_THE_OFFSET_(202, 1738)
    ATOM_SET_THE_OFFSET_(203, 1739)

    ATOM_SET_THE_OFFSET_(204, 1740)
    ATOM_SET_THE_OFFSET_(205, 1741)

    ATOM_SET_THE_OFFSET_(206, 1742)
    ATOM_SET_THE_OFFSET_(207, 1743)

    ATOM_SET_THE_OFFSET_(208, 1744)
    ATOM_SET_THE_OFFSET_(209, 1745)

    ATOM_SET_THE_OFFSET_(210, 1746)
    ATOM_SET_THE_OFFSET_(211, 1747)

    ATOM_SET_THE_OFFSET_(212, 1748)
    ATOM_SET_THE_OFFSET_(213, 1749)

    ATOM_SET_THE_OFFSET_(214, 1750)
    ATOM_SET_THE_OFFSET_(215, 1751)

    ATOM_SET_THE_OFFSET_(216, 1752)
    ATOM_SET_THE_OFFSET_(217, 1753)

    ATOM_SET_THE_OFFSET_(218, 1754)
    ATOM_SET_THE_OFFSET_(219, 1755)

    ATOM_SET_THE_OFFSET_(220, 1756)
    ATOM_SET_THE_OFFSET_(221, 1757)

    ATOM_SET_THE_OFFSET_(222, 1758)
    ATOM_SET_THE_OFFSET_(223, 1759)

    ATOM_SET_THE_OFFSET_(224, 1760)
    ATOM_SET_THE_OFFSET_(225, 1761)

    ATOM_SET_THE_OFFSET_(226, 1762)
    ATOM_SET_THE_OFFSET_(227, 1763)

    ATOM_SET_THE_OFFSET_(228, 1764)
    ATOM_SET_THE_OFFSET_(229, 1765)

    ATOM_SET_THE_OFFSET_(230, 1766)
    ATOM_SET_THE_OFFSET_(231, 1767)

    ATOM_SET_THE_OFFSET_(232, 1768)
    ATOM_SET_THE_OFFSET_(233, 1769)

    ATOM_SET_THE_OFFSET_(234, 1770)
    ATOM_SET_THE_OFFSET_(235, 1771)

    ATOM_SET_THE_OFFSET_(236, 1772)
    ATOM_SET_THE_OFFSET_(237, 1773)

    ATOM_SET_THE_OFFSET_(238, 1774)
    ATOM_SET_THE_OFFSET_(239, 1775)

    ATOM_SET_THE_OFFSET_(240, 1776)
    ATOM_SET_THE_OFFSET_(241, 1777)

    ATOM_SET_THE_OFFSET_(242, 1778)
    ATOM_SET_THE_OFFSET_(243, 1779)

    ATOM_SET_THE_OFFSET_(244, 1780)
    ATOM_SET_THE_OFFSET_(245, 1781)

    ATOM_SET_THE_OFFSET_(246, 1782)
    ATOM_SET_THE_OFFSET_(247, 1783)

    ATOM_SET_THE_OFFSET_(248, 1784)
    ATOM_SET_THE_OFFSET_(249, 1785)

    ATOM_SET_THE_OFFSET_(250, 1786)
    ATOM_SET_THE_OFFSET_(251, 1787)

    ATOM_SET_THE_OFFSET_(252, 1788)
    ATOM_SET_THE_OFFSET_(253, 1789)

    ATOM_SET_THE_OFFSET_(254, 1790)
    ATOM_SET_THE_OFFSET_(255, 1791)
    return arr;
}

consteval static auto offset_map7(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};

    ATOM_SET_THE_OFFSET_(0, 1792)
    ATOM_SET_THE_OFFSET_(1, 1793)
    ATOM_SET_THE_OFFSET_(2, 1794)
    ATOM_SET_THE_OFFSET_(3, 1795)

    ATOM_SET_THE_OFFSET_(4, 1796)
    ATOM_SET_THE_OFFSET_(5, 1797)

    ATOM_SET_THE_OFFSET_(6, 1798)
    ATOM_SET_THE_OFFSET_(7, 1799)

    ATOM_SET_THE_OFFSET_(8, 1800)
    ATOM_SET_THE_OFFSET_(9, 1801)

    ATOM_SET_THE_OFFSET_(10, 1802)
    ATOM_SET_THE_OFFSET_(11, 1803)

    ATOM_SET_THE_OFFSET_(12, 1804)
    ATOM_SET_THE_OFFSET_(13, 1805)

    ATOM_SET_THE_OFFSET_(14, 1806)
    ATOM_SET_THE_OFFSET_(15, 1807)

    ATOM_SET_THE_OFFSET_(16, 1808)
    ATOM_SET_THE_OFFSET_(17, 1809)

    ATOM_SET_THE_OFFSET_(18, 1810)
    ATOM_SET_THE_OFFSET_(19, 1811)

    ATOM_SET_THE_OFFSET_(20, 1812)
    ATOM_SET_THE_OFFSET_(21, 1813)

    ATOM_SET_THE_OFFSET_(22, 1814)
    ATOM_SET_THE_OFFSET_(23, 1815)

    ATOM_SET_THE_OFFSET_(24, 1816)
    ATOM_SET_THE_OFFSET_(25, 1817)

    ATOM_SET_THE_OFFSET_(26, 1818)
    ATOM_SET_THE_OFFSET_(27, 1819)

    ATOM_SET_THE_OFFSET_(28, 1820)
    ATOM_SET_THE_OFFSET_(29, 1821)

    ATOM_SET_THE_OFFSET_(30, 1822)
    ATOM_SET_THE_OFFSET_(31, 1823)

    ATOM_SET_THE_OFFSET_(32, 1824)
    ATOM_SET_THE_OFFSET_(33, 1825)

    ATOM_SET_THE_OFFSET_(34, 1826)
    ATOM_SET_THE_OFFSET_(35, 1827)

    ATOM_SET_THE_OFFSET_(36, 1828)
    ATOM_SET_THE_OFFSET_(37, 1829)

    ATOM_SET_THE_OFFSET_(38, 1830)
    ATOM_SET_THE_OFFSET_(39, 1831)

    ATOM_SET_THE_OFFSET_(40, 1832)
    ATOM_SET_THE_OFFSET_(41, 1833)

    ATOM_SET_THE_OFFSET_(42, 1834)
    ATOM_SET_THE_OFFSET_(43, 1835)

    ATOM_SET_THE_OFFSET_(44, 1836)
    ATOM_SET_THE_OFFSET_(45, 1837)

    ATOM_SET_THE_OFFSET_(46, 1838)
    ATOM_SET_THE_OFFSET_(47, 1839)

    ATOM_SET_THE_OFFSET_(48, 1840)
    ATOM_SET_THE_OFFSET_(49, 1841)

    ATOM_SET_THE_OFFSET_(50, 1842)
    ATOM_SET_THE_OFFSET_(51, 1843)

    ATOM_SET_THE_OFFSET_(52, 1844)
    ATOM_SET_THE_OFFSET_(53, 1845)

    ATOM_SET_THE_OFFSET_(54, 1846)
    ATOM_SET_THE_OFFSET_(55, 1847)

    ATOM_SET_THE_OFFSET_(56, 1848)
    ATOM_SET_THE_OFFSET_(57, 1849)

    ATOM_SET_THE_OFFSET_(58, 1850)
    ATOM_SET_THE_OFFSET_(59, 1851)

    ATOM_SET_THE_OFFSET_(60, 1852)
    ATOM_SET_THE_OFFSET_(61, 1853)

    ATOM_SET_THE_OFFSET_(62, 1854)
    ATOM_SET_THE_OFFSET_(63, 1855)

    ATOM_SET_THE_OFFSET_(64, 1856)
    ATOM_SET_THE_OFFSET_(65, 1857)

    ATOM_SET_THE_OFFSET_(66, 1858)
    ATOM_SET_THE_OFFSET_(67, 1859)

    ATOM_SET_THE_OFFSET_(68, 1860)
    ATOM_SET_THE_OFFSET_(69, 1861)

    ATOM_SET_THE_OFFSET_(70, 1862)
    ATOM_SET_THE_OFFSET_(71, 1863)

    ATOM_SET_THE_OFFSET_(72, 1864)
    ATOM_SET_THE_OFFSET_(73, 1865)

    ATOM_SET_THE_OFFSET_(74, 1866)
    ATOM_SET_THE_OFFSET_(75, 1867)

    ATOM_SET_THE_OFFSET_(76, 1868)
    ATOM_SET_THE_OFFSET_(77, 1869)

    ATOM_SET_THE_OFFSET_(78, 1870)
    ATOM_SET_THE_OFFSET_(79, 1871)

    ATOM_SET_THE_OFFSET_(80, 1872)
    ATOM_SET_THE_OFFSET_(81, 1873)

    ATOM_SET_THE_OFFSET_(82, 1874)
    ATOM_SET_THE_OFFSET_(83, 1875)

    ATOM_SET_THE_OFFSET_(84, 1876)
    ATOM_SET_THE_OFFSET_(85, 1877)

    ATOM_SET_THE_OFFSET_(86, 1878)
    ATOM_SET_THE_OFFSET_(87, 1879)

    ATOM_SET_THE_OFFSET_(88, 1880)
    ATOM_SET_THE_OFFSET_(89, 1881)

    ATOM_SET_THE_OFFSET_(90, 1882)
    ATOM_SET_THE_OFFSET_(91, 1883)

    ATOM_SET_THE_OFFSET_(92, 1884)
    ATOM_SET_THE_OFFSET_(93, 1885)

    ATOM_SET_THE_OFFSET_(94, 1886)
    ATOM_SET_THE_OFFSET_(95, 1887)

    ATOM_SET_THE_OFFSET_(96, 1888)
    ATOM_SET_THE_OFFSET_(97, 1889)

    ATOM_SET_THE_OFFSET_(98, 1890)
    ATOM_SET_THE_OFFSET_(99, 1891)

    ATOM_SET_THE_OFFSET_(100, 1892)
    ATOM_SET_THE_OFFSET_(101, 1893)

    ATOM_SET_THE_OFFSET_(102, 1894)
    ATOM_SET_THE_OFFSET_(103, 1895)

    ATOM_SET_THE_OFFSET_(104, 1896)
    ATOM_SET_THE_OFFSET_(105, 1897)

    ATOM_SET_THE_OFFSET_(106, 1898)
    ATOM_SET_THE_OFFSET_(107, 1899)

    ATOM_SET_THE_OFFSET_(108, 1900)
    ATOM_SET_THE_OFFSET_(109, 1901)

    ATOM_SET_THE_OFFSET_(110, 1902)
    ATOM_SET_THE_OFFSET_(111, 1903)

    ATOM_SET_THE_OFFSET_(112, 1904)
    ATOM_SET_THE_OFFSET_(113, 1905)

    ATOM_SET_THE_OFFSET_(114, 1906)
    ATOM_SET_THE_OFFSET_(115, 1907)

    ATOM_SET_THE_OFFSET_(116, 1908)
    ATOM_SET_THE_OFFSET_(117, 1909)

    ATOM_SET_THE_OFFSET_(118, 1910)
    ATOM_SET_THE_OFFSET_(119, 1911)

    ATOM_SET_THE_OFFSET_(120, 1912)
    ATOM_SET_THE_OFFSET_(121, 1913)

    ATOM_SET_THE_OFFSET_(122, 1914)
    ATOM_SET_THE_OFFSET_(123, 1915)

    ATOM_SET_THE_OFFSET_(124, 1916)
    ATOM_SET_THE_OFFSET_(125, 1917)

    ATOM_SET_THE_OFFSET_(126, 1918)
    ATOM_SET_THE_OFFSET_(127, 1919)

    ATOM_SET_THE_OFFSET_(128, 1920)
    ATOM_SET_THE_OFFSET_(129, 1921)

    ATOM_SET_THE_OFFSET_(130, 1922)
    ATOM_SET_THE_OFFSET_(131, 1923)

    ATOM_SET_THE_OFFSET_(132, 1924)
    ATOM_SET_THE_OFFSET_(133, 1925)

    ATOM_SET_THE_OFFSET_(134, 1926)
    ATOM_SET_THE_OFFSET_(135, 1927)

    ATOM_SET_THE_OFFSET_(136, 1928)
    ATOM_SET_THE_OFFSET_(137, 1929)

    ATOM_SET_THE_OFFSET_(138, 1930)
    ATOM_SET_THE_OFFSET_(139, 1931)

    ATOM_SET_THE_OFFSET_(140, 1932)
    ATOM_SET_THE_OFFSET_(141, 1933)

    ATOM_SET_THE_OFFSET_(142, 1934)
    ATOM_SET_THE_OFFSET_(143, 1935)

    ATOM_SET_THE_OFFSET_(144, 1936)
    ATOM_SET_THE_OFFSET_(145, 1937)

    ATOM_SET_THE_OFFSET_(146, 1938)
    ATOM_SET_THE_OFFSET_(147, 1939)

    ATOM_SET_THE_OFFSET_(148, 1940)
    ATOM_SET_THE_OFFSET_(149, 1941)

    ATOM_SET_THE_OFFSET_(150, 1942)
    ATOM_SET_THE_OFFSET_(151, 1943)

    ATOM_SET_THE_OFFSET_(152, 1944)
    ATOM_SET_THE_OFFSET_(153, 1945)

    ATOM_SET_THE_OFFSET_(154, 1946)
    ATOM_SET_THE_OFFSET_(155, 1947)

    ATOM_SET_THE_OFFSET_(156, 1948)
    ATOM_SET_THE_OFFSET_(157, 1949)

    ATOM_SET_THE_OFFSET_(158, 1950)
    ATOM_SET_THE_OFFSET_(159, 1951)

    ATOM_SET_THE_OFFSET_(160, 1952)
    ATOM_SET_THE_OFFSET_(161, 1953)

    ATOM_SET_THE_OFFSET_(162, 1954)
    ATOM_SET_THE_OFFSET_(163, 1955)

    ATOM_SET_THE_OFFSET_(164, 1956)
    ATOM_SET_THE_OFFSET_(165, 1957)

    ATOM_SET_THE_OFFSET_(166, 1958)
    ATOM_SET_THE_OFFSET_(167, 1959)

    ATOM_SET_THE_OFFSET_(168, 1960)
    ATOM_SET_THE_OFFSET_(169, 1961)

    ATOM_SET_THE_OFFSET_(170, 1962)
    ATOM_SET_THE_OFFSET_(171, 1963)

    ATOM_SET_THE_OFFSET_(172, 1964)
    ATOM_SET_THE_OFFSET_(173, 1965)

    ATOM_SET_THE_OFFSET_(174, 1966)
    ATOM_SET_THE_OFFSET_(175, 1967)

    ATOM_SET_THE_OFFSET_(176, 1968)
    ATOM_SET_THE_OFFSET_(177, 1969)

    ATOM_SET_THE_OFFSET_(178, 1970)
    ATOM_SET_THE_OFFSET_(179, 1971)

    ATOM_SET_THE_OFFSET_(180, 1972)
    ATOM_SET_THE_OFFSET_(181, 1973)

    ATOM_SET_THE_OFFSET_(182, 1974)
    ATOM_SET_THE_OFFSET_(183, 1975)

    ATOM_SET_THE_OFFSET_(184, 1976)
    ATOM_SET_THE_OFFSET_(185, 1977)

    ATOM_SET_THE_OFFSET_(186, 1978)
    ATOM_SET_THE_OFFSET_(187, 1979)

    ATOM_SET_THE_OFFSET_(188, 1980)
    ATOM_SET_THE_OFFSET_(189, 1981)

    ATOM_SET_THE_OFFSET_(190, 1982)
    ATOM_SET_THE_OFFSET_(191, 1983)

    ATOM_SET_THE_OFFSET_(192, 1984)
    ATOM_SET_THE_OFFSET_(193, 1985)

    ATOM_SET_THE_OFFSET_(194, 1986)
    ATOM_SET_THE_OFFSET_(195, 1987)

    ATOM_SET_THE_OFFSET_(196, 1988)
    ATOM_SET_THE_OFFSET_(197, 1989)

    ATOM_SET_THE_OFFSET_(198, 1990)
    ATOM_SET_THE_OFFSET_(199, 1991)

    ATOM_SET_THE_OFFSET_(200, 1992)
    ATOM_SET_THE_OFFSET_(201, 1993)

    ATOM_SET_THE_OFFSET_(202, 1994)
    ATOM_SET_THE_OFFSET_(203, 1995)

    ATOM_SET_THE_OFFSET_(204, 1996)
    ATOM_SET_THE_OFFSET_(205, 1997)

    ATOM_SET_THE_OFFSET_(206, 1998)
    ATOM_SET_THE_OFFSET_(207, 1999)

    ATOM_SET_THE_OFFSET_(208, 2000)
    ATOM_SET_THE_OFFSET_(209, 2001)

    ATOM_SET_THE_OFFSET_(210, 2002)
    ATOM_SET_THE_OFFSET_(211, 2003)

    ATOM_SET_THE_OFFSET_(212, 2004)
    ATOM_SET_THE_OFFSET_(213, 2005)

    ATOM_SET_THE_OFFSET_(214, 2006)
    ATOM_SET_THE_OFFSET_(215, 2007)

    ATOM_SET_THE_OFFSET_(216, 2008)
    ATOM_SET_THE_OFFSET_(217, 2009)

    ATOM_SET_THE_OFFSET_(218, 2010)
    ATOM_SET_THE_OFFSET_(219, 2011)

    ATOM_SET_THE_OFFSET_(220, 2012)
    ATOM_SET_THE_OFFSET_(221, 2013)

    ATOM_SET_THE_OFFSET_(222, 2014)
    ATOM_SET_THE_OFFSET_(223, 2015)

    ATOM_SET_THE_OFFSET_(224, 2016)
    ATOM_SET_THE_OFFSET_(225, 2017)

    ATOM_SET_THE_OFFSET_(226, 2018)
    ATOM_SET_THE_OFFSET_(227, 2019)

    ATOM_SET_THE_OFFSET_(228, 2020)
    ATOM_SET_THE_OFFSET_(229, 2021)

    ATOM_SET_THE_OFFSET_(230, 2022)
    ATOM_SET_THE_OFFSET_(231, 2023)

    ATOM_SET_THE_OFFSET_(232, 2024)
    ATOM_SET_THE_OFFSET_(233, 2025)

    ATOM_SET_THE_OFFSET_(234, 2026)
    ATOM_SET_THE_OFFSET_(235, 2027)

    ATOM_SET_THE_OFFSET_(236, 2028)
    ATOM_SET_THE_OFFSET_(237, 2029)

    ATOM_SET_THE_OFFSET_(238, 2030)
    ATOM_SET_THE_OFFSET_(239, 2031)

    ATOM_SET_THE_OFFSET_(240, 2032)
    ATOM_SET_THE_OFFSET_(241, 2033)

    ATOM_SET_THE_OFFSET_(242, 2034)
    ATOM_SET_THE_OFFSET_(243, 2035)

    ATOM_SET_THE_OFFSET_(244, 2036)
    ATOM_SET_THE_OFFSET_(245, 2037)

    ATOM_SET_THE_OFFSET_(246, 2038)
    ATOM_SET_THE_OFFSET_(247, 2039)

    ATOM_SET_THE_OFFSET_(248, 2040)
    ATOM_SET_THE_OFFSET_(249, 2041)

    ATOM_SET_THE_OFFSET_(250, 2042)
    ATOM_SET_THE_OFFSET_(251, 2043)

    ATOM_SET_THE_OFFSET_(252, 2044)
    ATOM_SET_THE_OFFSET_(253, 2045)

    ATOM_SET_THE_OFFSET_(254, 2046)
    ATOM_SET_THE_OFFSET_(255, 2047)
    return arr;
}

consteval static auto offset_map8(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 2048)
    ATOM_SET_THE_OFFSET_(1, 2049)
    ATOM_SET_THE_OFFSET_(2, 2050)
    ATOM_SET_THE_OFFSET_(3, 2051)
    ATOM_SET_THE_OFFSET_(4, 2052)
    ATOM_SET_THE_OFFSET_(5, 2053)
    ATOM_SET_THE_OFFSET_(6, 2054)
    ATOM_SET_THE_OFFSET_(7, 2055)
    ATOM_SET_THE_OFFSET_(8, 2056)
    ATOM_SET_THE_OFFSET_(9, 2057)
    ATOM_SET_THE_OFFSET_(10, 2058)
    ATOM_SET_THE_OFFSET_(11, 2059)
    ATOM_SET_THE_OFFSET_(12, 2060)
    ATOM_SET_THE_OFFSET_(13, 2061)
    ATOM_SET_THE_OFFSET_(14, 2062)
    ATOM_SET_THE_OFFSET_(15, 2063)
    ATOM_SET_THE_OFFSET_(16, 2064)
    ATOM_SET_THE_OFFSET_(17, 2065)
    ATOM_SET_THE_OFFSET_(18, 2066)
    ATOM_SET_THE_OFFSET_(19, 2067)
    ATOM_SET_THE_OFFSET_(20, 2068)
    ATOM_SET_THE_OFFSET_(21, 2069)
    ATOM_SET_THE_OFFSET_(22, 2070)
    ATOM_SET_THE_OFFSET_(23, 2071)
    ATOM_SET_THE_OFFSET_(24, 2072)
    ATOM_SET_THE_OFFSET_(25, 2073)
    ATOM_SET_THE_OFFSET_(26, 2074)
    ATOM_SET_THE_OFFSET_(27, 2075)
    ATOM_SET_THE_OFFSET_(28, 2076)
    ATOM_SET_THE_OFFSET_(29, 2077)
    ATOM_SET_THE_OFFSET_(30, 2078)
    ATOM_SET_THE_OFFSET_(31, 2079)
    ATOM_SET_THE_OFFSET_(32, 2080)
    ATOM_SET_THE_OFFSET_(33, 2081)
    ATOM_SET_THE_OFFSET_(34, 2082)
    ATOM_SET_THE_OFFSET_(35, 2083)
    ATOM_SET_THE_OFFSET_(36, 2084)
    ATOM_SET_THE_OFFSET_(37, 2085)
    ATOM_SET_THE_OFFSET_(38, 2086)
    ATOM_SET_THE_OFFSET_(39, 2087)
    ATOM_SET_THE_OFFSET_(40, 2088)
    ATOM_SET_THE_OFFSET_(41, 2089)
    ATOM_SET_THE_OFFSET_(42, 2090)
    ATOM_SET_THE_OFFSET_(43, 2091)
    ATOM_SET_THE_OFFSET_(44, 2092)
    ATOM_SET_THE_OFFSET_(45, 2093)
    ATOM_SET_THE_OFFSET_(46, 2094)
    ATOM_SET_THE_OFFSET_(47, 2095)
    ATOM_SET_THE_OFFSET_(48, 2096)
    ATOM_SET_THE_OFFSET_(49, 2097)
    ATOM_SET_THE_OFFSET_(50, 2098)
    ATOM_SET_THE_OFFSET_(51, 2099)
    ATOM_SET_THE_OFFSET_(52, 2100)
    ATOM_SET_THE_OFFSET_(53, 2101)
    ATOM_SET_THE_OFFSET_(54, 2102)
    ATOM_SET_THE_OFFSET_(55, 2103)
    ATOM_SET_THE_OFFSET_(56, 2104)
    ATOM_SET_THE_OFFSET_(57, 2105)
    ATOM_SET_THE_OFFSET_(58, 2106)
    ATOM_SET_THE_OFFSET_(59, 2107)
    ATOM_SET_THE_OFFSET_(60, 2108)
    ATOM_SET_THE_OFFSET_(61, 2109)
    ATOM_SET_THE_OFFSET_(62, 2110)
    ATOM_SET_THE_OFFSET_(63, 2111)
    ATOM_SET_THE_OFFSET_(64, 2112)
    ATOM_SET_THE_OFFSET_(65, 2113)
    ATOM_SET_THE_OFFSET_(66, 2114)
    ATOM_SET_THE_OFFSET_(67, 2115)
    ATOM_SET_THE_OFFSET_(68, 2116)
    ATOM_SET_THE_OFFSET_(69, 2117)
    ATOM_SET_THE_OFFSET_(70, 2118)
    ATOM_SET_THE_OFFSET_(71, 2119)
    ATOM_SET_THE_OFFSET_(72, 2120)
    ATOM_SET_THE_OFFSET_(73, 2121)
    ATOM_SET_THE_OFFSET_(74, 2122)
    ATOM_SET_THE_OFFSET_(75, 2123)
    ATOM_SET_THE_OFFSET_(76, 2124)
    ATOM_SET_THE_OFFSET_(77, 2125)
    ATOM_SET_THE_OFFSET_(78, 2126)
    ATOM_SET_THE_OFFSET_(79, 2127)
    ATOM_SET_THE_OFFSET_(80, 2128)
    ATOM_SET_THE_OFFSET_(81, 2129)
    ATOM_SET_THE_OFFSET_(82, 2130)
    ATOM_SET_THE_OFFSET_(83, 2131)
    ATOM_SET_THE_OFFSET_(84, 2132)
    ATOM_SET_THE_OFFSET_(85, 2133)
    ATOM_SET_THE_OFFSET_(86, 2134)
    ATOM_SET_THE_OFFSET_(87, 2135)
    ATOM_SET_THE_OFFSET_(88, 2136)
    ATOM_SET_THE_OFFSET_(89, 2137)
    ATOM_SET_THE_OFFSET_(90, 2138)
    ATOM_SET_THE_OFFSET_(91, 2139)
    ATOM_SET_THE_OFFSET_(92, 2140)
    ATOM_SET_THE_OFFSET_(93, 2141)
    ATOM_SET_THE_OFFSET_(94, 2142)
    ATOM_SET_THE_OFFSET_(95, 2143)
    ATOM_SET_THE_OFFSET_(96, 2144)
    ATOM_SET_THE_OFFSET_(97, 2145)
    ATOM_SET_THE_OFFSET_(98, 2146)
    ATOM_SET_THE_OFFSET_(99, 2147)
    ATOM_SET_THE_OFFSET_(100, 2148)
    ATOM_SET_THE_OFFSET_(101, 2149)
    ATOM_SET_THE_OFFSET_(102, 2150)
    ATOM_SET_THE_OFFSET_(103, 2151)
    ATOM_SET_THE_OFFSET_(104, 2152)
    ATOM_SET_THE_OFFSET_(105, 2153)
    ATOM_SET_THE_OFFSET_(106, 2154)
    ATOM_SET_THE_OFFSET_(107, 2155)
    ATOM_SET_THE_OFFSET_(108, 2156)
    ATOM_SET_THE_OFFSET_(109, 2157)
    ATOM_SET_THE_OFFSET_(110, 2158)
    ATOM_SET_THE_OFFSET_(111, 2159)
    ATOM_SET_THE_OFFSET_(112, 2160)
    ATOM_SET_THE_OFFSET_(113, 2161)
    ATOM_SET_THE_OFFSET_(114, 2162)
    ATOM_SET_THE_OFFSET_(115, 2163)
    ATOM_SET_THE_OFFSET_(116, 2164)
    ATOM_SET_THE_OFFSET_(117, 2165)
    ATOM_SET_THE_OFFSET_(118, 2166)
    ATOM_SET_THE_OFFSET_(119, 2167)
    ATOM_SET_THE_OFFSET_(120, 2168)
    ATOM_SET_THE_OFFSET_(121, 2169)
    ATOM_SET_THE_OFFSET_(122, 2170)
    ATOM_SET_THE_OFFSET_(123, 2171)
    ATOM_SET_THE_OFFSET_(124, 2172)
    ATOM_SET_THE_OFFSET_(125, 2173)
    ATOM_SET_THE_OFFSET_(126, 2174)
    ATOM_SET_THE_OFFSET_(127, 2175)
    ATOM_SET_THE_OFFSET_(128, 2176)
    ATOM_SET_THE_OFFSET_(129, 2177)
    ATOM_SET_THE_OFFSET_(130, 2178)
    ATOM_SET_THE_OFFSET_(131, 2179)
    ATOM_SET_THE_OFFSET_(132, 2180)
    ATOM_SET_THE_OFFSET_(133, 2181)
    ATOM_SET_THE_OFFSET_(134, 2182)
    ATOM_SET_THE_OFFSET_(135, 2183)
    ATOM_SET_THE_OFFSET_(136, 2184)
    ATOM_SET_THE_OFFSET_(137, 2185)
    ATOM_SET_THE_OFFSET_(138, 2186)
    ATOM_SET_THE_OFFSET_(139, 2187)
    ATOM_SET_THE_OFFSET_(140, 2188)
    ATOM_SET_THE_OFFSET_(141, 2189)
    ATOM_SET_THE_OFFSET_(142, 2190)
    ATOM_SET_THE_OFFSET_(143, 2191)
    ATOM_SET_THE_OFFSET_(144, 2192)
    ATOM_SET_THE_OFFSET_(145, 2193)
    ATOM_SET_THE_OFFSET_(146, 2194)
    ATOM_SET_THE_OFFSET_(147, 2195)
    ATOM_SET_THE_OFFSET_(148, 2196)
    ATOM_SET_THE_OFFSET_(149, 2197)
    ATOM_SET_THE_OFFSET_(150, 2198)
    ATOM_SET_THE_OFFSET_(151, 2199)
    ATOM_SET_THE_OFFSET_(152, 2200)
    ATOM_SET_THE_OFFSET_(153, 2201)
    ATOM_SET_THE_OFFSET_(154, 2202)
    ATOM_SET_THE_OFFSET_(155, 2203)
    ATOM_SET_THE_OFFSET_(156, 2204)
    ATOM_SET_THE_OFFSET_(157, 2205)
    ATOM_SET_THE_OFFSET_(158, 2206)
    ATOM_SET_THE_OFFSET_(159, 2207)
    ATOM_SET_THE_OFFSET_(160, 2208)
    ATOM_SET_THE_OFFSET_(161, 2209)
    ATOM_SET_THE_OFFSET_(162, 2210)
    ATOM_SET_THE_OFFSET_(163, 2211)
    ATOM_SET_THE_OFFSET_(164, 2212)
    ATOM_SET_THE_OFFSET_(165, 2213)
    ATOM_SET_THE_OFFSET_(166, 2214)
    ATOM_SET_THE_OFFSET_(167, 2215)
    ATOM_SET_THE_OFFSET_(168, 2216)
    ATOM_SET_THE_OFFSET_(169, 2217)
    ATOM_SET_THE_OFFSET_(170, 2218)
    ATOM_SET_THE_OFFSET_(171, 2219)
    ATOM_SET_THE_OFFSET_(172, 2220)
    ATOM_SET_THE_OFFSET_(173, 2221)
    ATOM_SET_THE_OFFSET_(174, 2222)
    ATOM_SET_THE_OFFSET_(175, 2223)
    ATOM_SET_THE_OFFSET_(176, 2224)
    ATOM_SET_THE_OFFSET_(177, 2225)
    ATOM_SET_THE_OFFSET_(178, 2226)
    ATOM_SET_THE_OFFSET_(179, 2227)
    ATOM_SET_THE_OFFSET_(180, 2228)
    ATOM_SET_THE_OFFSET_(181, 2229)
    ATOM_SET_THE_OFFSET_(182, 2230)
    ATOM_SET_THE_OFFSET_(183, 2231)
    ATOM_SET_THE_OFFSET_(184, 2232)
    ATOM_SET_THE_OFFSET_(185, 2233)
    ATOM_SET_THE_OFFSET_(186, 2234)
    ATOM_SET_THE_OFFSET_(187, 2235)
    ATOM_SET_THE_OFFSET_(188, 2236)
    ATOM_SET_THE_OFFSET_(189, 2237)
    ATOM_SET_THE_OFFSET_(190, 2238)
    ATOM_SET_THE_OFFSET_(191, 2239)
    ATOM_SET_THE_OFFSET_(192, 2240)
    ATOM_SET_THE_OFFSET_(193, 2241)
    ATOM_SET_THE_OFFSET_(194, 2242)
    ATOM_SET_THE_OFFSET_(195, 2243)
    ATOM_SET_THE_OFFSET_(196, 2244)
    ATOM_SET_THE_OFFSET_(197, 2245)
    ATOM_SET_THE_OFFSET_(198, 2246)
    ATOM_SET_THE_OFFSET_(199, 2247)
    ATOM_SET_THE_OFFSET_(200, 2248)
    ATOM_SET_THE_OFFSET_(201, 2249)
    ATOM_SET_THE_OFFSET_(202, 2250)
    ATOM_SET_THE_OFFSET_(203, 2251)
    ATOM_SET_THE_OFFSET_(204, 2252)
    ATOM_SET_THE_OFFSET_(205, 2253)
    ATOM_SET_THE_OFFSET_(206, 2254)
    ATOM_SET_THE_OFFSET_(207, 2255)
    ATOM_SET_THE_OFFSET_(208, 2256)
    ATOM_SET_THE_OFFSET_(209, 2257)
    ATOM_SET_THE_OFFSET_(210, 2258)
    ATOM_SET_THE_OFFSET_(211, 2259)
    ATOM_SET_THE_OFFSET_(212, 2260)
    ATOM_SET_THE_OFFSET_(213, 2261)
    ATOM_SET_THE_OFFSET_(214, 2262)
    ATOM_SET_THE_OFFSET_(215, 2263)
    ATOM_SET_THE_OFFSET_(216, 2264)
    ATOM_SET_THE_OFFSET_(217, 2265)
    ATOM_SET_THE_OFFSET_(218, 2266)
    ATOM_SET_THE_OFFSET_(219, 2267)
    ATOM_SET_THE_OFFSET_(220, 2268)
    ATOM_SET_THE_OFFSET_(221, 2269)
    ATOM_SET_THE_OFFSET_(222, 2270)
    ATOM_SET_THE_OFFSET_(223, 2271)
    ATOM_SET_THE_OFFSET_(224, 2272)
    ATOM_SET_THE_OFFSET_(225, 2273)
    ATOM_SET_THE_OFFSET_(226, 2274)
    ATOM_SET_THE_OFFSET_(227, 2275)
    ATOM_SET_THE_OFFSET_(228, 2276)
    ATOM_SET_THE_OFFSET_(229, 2277)
    ATOM_SET_THE_OFFSET_(230, 2278)
    ATOM_SET_THE_OFFSET_(231, 2279)
    ATOM_SET_THE_OFFSET_(232, 2280)
    ATOM_SET_THE_OFFSET_(233, 2281)
    ATOM_SET_THE_OFFSET_(234, 2282)
    ATOM_SET_THE_OFFSET_(235, 2283)
    ATOM_SET_THE_OFFSET_(236, 2284)
    ATOM_SET_THE_OFFSET_(237, 2285)
    ATOM_SET_THE_OFFSET_(238, 2286)
    ATOM_SET_THE_OFFSET_(239, 2287)
    ATOM_SET_THE_OFFSET_(240, 2288)
    ATOM_SET_THE_OFFSET_(241, 2289)
    ATOM_SET_THE_OFFSET_(242, 2290)
    ATOM_SET_THE_OFFSET_(243, 2291)
    ATOM_SET_THE_OFFSET_(244, 2292)
    ATOM_SET_THE_OFFSET_(245, 2293)
    ATOM_SET_THE_OFFSET_(246, 2294)
    ATOM_SET_THE_OFFSET_(247, 2295)
    ATOM_SET_THE_OFFSET_(248, 2296)
    ATOM_SET_THE_OFFSET_(249, 2297)
    ATOM_SET_THE_OFFSET_(250, 2298)
    ATOM_SET_THE_OFFSET_(251, 2299)
    ATOM_SET_THE_OFFSET_(252, 2300)
    ATOM_SET_THE_OFFSET_(253, 2301)
    ATOM_SET_THE_OFFSET_(254, 2302)
    ATOM_SET_THE_OFFSET_(255, 2303)
    return arr;
}

consteval static auto offset_map9(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 2304)
    ATOM_SET_THE_OFFSET_(1, 2305)
    ATOM_SET_THE_OFFSET_(2, 2306)
    ATOM_SET_THE_OFFSET_(3, 2307)
    ATOM_SET_THE_OFFSET_(4, 2308)
    ATOM_SET_THE_OFFSET_(5, 2309)
    ATOM_SET_THE_OFFSET_(6, 2310)
    ATOM_SET_THE_OFFSET_(7, 2311)
    ATOM_SET_THE_OFFSET_(8, 2312)
    ATOM_SET_THE_OFFSET_(9, 2313)
    ATOM_SET_THE_OFFSET_(10, 2314)
    ATOM_SET_THE_OFFSET_(11, 2315)
    ATOM_SET_THE_OFFSET_(12, 2316)
    ATOM_SET_THE_OFFSET_(13, 2317)
    ATOM_SET_THE_OFFSET_(14, 2318)
    ATOM_SET_THE_OFFSET_(15, 2319)
    ATOM_SET_THE_OFFSET_(16, 2320)
    ATOM_SET_THE_OFFSET_(17, 2321)
    ATOM_SET_THE_OFFSET_(18, 2322)
    ATOM_SET_THE_OFFSET_(19, 2323)
    ATOM_SET_THE_OFFSET_(20, 2324)
    ATOM_SET_THE_OFFSET_(21, 2325)
    ATOM_SET_THE_OFFSET_(22, 2326)
    ATOM_SET_THE_OFFSET_(23, 2327)
    ATOM_SET_THE_OFFSET_(24, 2328)
    ATOM_SET_THE_OFFSET_(25, 2329)
    ATOM_SET_THE_OFFSET_(26, 2330)
    ATOM_SET_THE_OFFSET_(27, 2331)
    ATOM_SET_THE_OFFSET_(28, 2332)
    ATOM_SET_THE_OFFSET_(29, 2333)
    ATOM_SET_THE_OFFSET_(30, 2334)
    ATOM_SET_THE_OFFSET_(31, 2335)
    ATOM_SET_THE_OFFSET_(32, 2336)
    ATOM_SET_THE_OFFSET_(33, 2337)
    ATOM_SET_THE_OFFSET_(34, 2338)
    ATOM_SET_THE_OFFSET_(35, 2339)
    ATOM_SET_THE_OFFSET_(36, 2340)
    ATOM_SET_THE_OFFSET_(37, 2341)
    ATOM_SET_THE_OFFSET_(38, 2342)
    ATOM_SET_THE_OFFSET_(39, 2343)
    ATOM_SET_THE_OFFSET_(40, 2344)
    ATOM_SET_THE_OFFSET_(41, 2345)
    ATOM_SET_THE_OFFSET_(42, 2346)
    ATOM_SET_THE_OFFSET_(43, 2347)
    ATOM_SET_THE_OFFSET_(44, 2348)
    ATOM_SET_THE_OFFSET_(45, 2349)
    ATOM_SET_THE_OFFSET_(46, 2350)
    ATOM_SET_THE_OFFSET_(47, 2351)
    ATOM_SET_THE_OFFSET_(48, 2352)
    ATOM_SET_THE_OFFSET_(49, 2353)
    ATOM_SET_THE_OFFSET_(50, 2354)
    ATOM_SET_THE_OFFSET_(51, 2355)
    ATOM_SET_THE_OFFSET_(52, 2356)
    ATOM_SET_THE_OFFSET_(53, 2357)
    ATOM_SET_THE_OFFSET_(54, 2358)
    ATOM_SET_THE_OFFSET_(55, 2359)
    ATOM_SET_THE_OFFSET_(56, 2360)
    ATOM_SET_THE_OFFSET_(57, 2361)
    ATOM_SET_THE_OFFSET_(58, 2362)
    ATOM_SET_THE_OFFSET_(59, 2363)
    ATOM_SET_THE_OFFSET_(60, 2364)
    ATOM_SET_THE_OFFSET_(61, 2365)
    ATOM_SET_THE_OFFSET_(62, 2366)
    ATOM_SET_THE_OFFSET_(63, 2367)
    ATOM_SET_THE_OFFSET_(64, 2368)
    ATOM_SET_THE_OFFSET_(65, 2369)
    ATOM_SET_THE_OFFSET_(66, 2370)
    ATOM_SET_THE_OFFSET_(67, 2371)
    ATOM_SET_THE_OFFSET_(68, 2372)
    ATOM_SET_THE_OFFSET_(69, 2373)
    ATOM_SET_THE_OFFSET_(70, 2374)
    ATOM_SET_THE_OFFSET_(71, 2375)
    ATOM_SET_THE_OFFSET_(72, 2376)
    ATOM_SET_THE_OFFSET_(73, 2377)
    ATOM_SET_THE_OFFSET_(74, 2378)
    ATOM_SET_THE_OFFSET_(75, 2379)
    ATOM_SET_THE_OFFSET_(76, 2380)
    ATOM_SET_THE_OFFSET_(77, 2381)
    ATOM_SET_THE_OFFSET_(78, 2382)
    ATOM_SET_THE_OFFSET_(79, 2383)
    ATOM_SET_THE_OFFSET_(80, 2384)
    ATOM_SET_THE_OFFSET_(81, 2385)
    ATOM_SET_THE_OFFSET_(82, 2386)
    ATOM_SET_THE_OFFSET_(83, 2387)
    ATOM_SET_THE_OFFSET_(84, 2388)
    ATOM_SET_THE_OFFSET_(85, 2389)
    ATOM_SET_THE_OFFSET_(86, 2390)
    ATOM_SET_THE_OFFSET_(87, 2391)
    ATOM_SET_THE_OFFSET_(88, 2392)
    ATOM_SET_THE_OFFSET_(89, 2393)
    ATOM_SET_THE_OFFSET_(90, 2394)
    ATOM_SET_THE_OFFSET_(91, 2395)
    ATOM_SET_THE_OFFSET_(92, 2396)
    ATOM_SET_THE_OFFSET_(93, 2397)
    ATOM_SET_THE_OFFSET_(94, 2398)
    ATOM_SET_THE_OFFSET_(95, 2399)
    ATOM_SET_THE_OFFSET_(96, 2400)
    ATOM_SET_THE_OFFSET_(97, 2401)
    ATOM_SET_THE_OFFSET_(98, 2402)
    ATOM_SET_THE_OFFSET_(99, 2403)
    ATOM_SET_THE_OFFSET_(100, 2404)
    ATOM_SET_THE_OFFSET_(101, 2405)
    ATOM_SET_THE_OFFSET_(102, 2406)
    ATOM_SET_THE_OFFSET_(103, 2407)
    ATOM_SET_THE_OFFSET_(104, 2408)
    ATOM_SET_THE_OFFSET_(105, 2409)
    ATOM_SET_THE_OFFSET_(106, 2410)
    ATOM_SET_THE_OFFSET_(107, 2411)
    ATOM_SET_THE_OFFSET_(108, 2412)
    ATOM_SET_THE_OFFSET_(109, 2413)
    ATOM_SET_THE_OFFSET_(110, 2414)
    ATOM_SET_THE_OFFSET_(111, 2415)
    ATOM_SET_THE_OFFSET_(112, 2416)
    ATOM_SET_THE_OFFSET_(113, 2417)
    ATOM_SET_THE_OFFSET_(114, 2418)
    ATOM_SET_THE_OFFSET_(115, 2419)
    ATOM_SET_THE_OFFSET_(116, 2420)
    ATOM_SET_THE_OFFSET_(117, 2421)
    ATOM_SET_THE_OFFSET_(118, 2422)
    ATOM_SET_THE_OFFSET_(119, 2423)
    ATOM_SET_THE_OFFSET_(120, 2424)
    ATOM_SET_THE_OFFSET_(121, 2425)
    ATOM_SET_THE_OFFSET_(122, 2426)
    ATOM_SET_THE_OFFSET_(123, 2427)
    ATOM_SET_THE_OFFSET_(124, 2428)
    ATOM_SET_THE_OFFSET_(125, 2429)
    ATOM_SET_THE_OFFSET_(126, 2430)
    ATOM_SET_THE_OFFSET_(127, 2431)
    ATOM_SET_THE_OFFSET_(128, 2432)
    ATOM_SET_THE_OFFSET_(129, 2433)
    ATOM_SET_THE_OFFSET_(130, 2434)
    ATOM_SET_THE_OFFSET_(131, 2435)
    ATOM_SET_THE_OFFSET_(132, 2436)
    ATOM_SET_THE_OFFSET_(133, 2437)
    ATOM_SET_THE_OFFSET_(134, 2438)
    ATOM_SET_THE_OFFSET_(135, 2439)
    ATOM_SET_THE_OFFSET_(136, 2440)
    ATOM_SET_THE_OFFSET_(137, 2441)
    ATOM_SET_THE_OFFSET_(138, 2442)
    ATOM_SET_THE_OFFSET_(139, 2443)
    ATOM_SET_THE_OFFSET_(140, 2444)
    ATOM_SET_THE_OFFSET_(141, 2445)
    ATOM_SET_THE_OFFSET_(142, 2446)
    ATOM_SET_THE_OFFSET_(143, 2447)
    ATOM_SET_THE_OFFSET_(144, 2448)
    ATOM_SET_THE_OFFSET_(145, 2449)
    ATOM_SET_THE_OFFSET_(146, 2450)
    ATOM_SET_THE_OFFSET_(147, 2451)
    ATOM_SET_THE_OFFSET_(148, 2452)
    ATOM_SET_THE_OFFSET_(149, 2453)
    ATOM_SET_THE_OFFSET_(150, 2454)
    ATOM_SET_THE_OFFSET_(151, 2455)
    ATOM_SET_THE_OFFSET_(152, 2456)
    ATOM_SET_THE_OFFSET_(153, 2457)
    ATOM_SET_THE_OFFSET_(154, 2458)
    ATOM_SET_THE_OFFSET_(155, 2459)
    ATOM_SET_THE_OFFSET_(156, 2460)
    ATOM_SET_THE_OFFSET_(157, 2461)
    ATOM_SET_THE_OFFSET_(158, 2462)
    ATOM_SET_THE_OFFSET_(159, 2463)
    ATOM_SET_THE_OFFSET_(160, 2464)
    ATOM_SET_THE_OFFSET_(161, 2465)
    ATOM_SET_THE_OFFSET_(162, 2466)
    ATOM_SET_THE_OFFSET_(163, 2467)
    ATOM_SET_THE_OFFSET_(164, 2468)
    ATOM_SET_THE_OFFSET_(165, 2469)
    ATOM_SET_THE_OFFSET_(166, 2470)
    ATOM_SET_THE_OFFSET_(167, 2471)
    ATOM_SET_THE_OFFSET_(168, 2472)
    ATOM_SET_THE_OFFSET_(169, 2473)
    ATOM_SET_THE_OFFSET_(170, 2474)
    ATOM_SET_THE_OFFSET_(171, 2475)
    ATOM_SET_THE_OFFSET_(172, 2476)
    ATOM_SET_THE_OFFSET_(173, 2477)
    ATOM_SET_THE_OFFSET_(174, 2478)
    ATOM_SET_THE_OFFSET_(175, 2479)
    ATOM_SET_THE_OFFSET_(176, 2480)
    ATOM_SET_THE_OFFSET_(177, 2481)
    ATOM_SET_THE_OFFSET_(178, 2482)
    ATOM_SET_THE_OFFSET_(179, 2483)
    ATOM_SET_THE_OFFSET_(180, 2484)
    ATOM_SET_THE_OFFSET_(181, 2485)
    ATOM_SET_THE_OFFSET_(182, 2486)
    ATOM_SET_THE_OFFSET_(183, 2487)
    ATOM_SET_THE_OFFSET_(184, 2488)
    ATOM_SET_THE_OFFSET_(185, 2489)
    ATOM_SET_THE_OFFSET_(186, 2490)
    ATOM_SET_THE_OFFSET_(187, 2491)
    ATOM_SET_THE_OFFSET_(188, 2492)
    ATOM_SET_THE_OFFSET_(189, 2493)
    ATOM_SET_THE_OFFSET_(190, 2494)
    ATOM_SET_THE_OFFSET_(191, 2495)
    ATOM_SET_THE_OFFSET_(192, 2496)
    ATOM_SET_THE_OFFSET_(193, 2497)
    ATOM_SET_THE_OFFSET_(194, 2498)
    ATOM_SET_THE_OFFSET_(195, 2499)
    ATOM_SET_THE_OFFSET_(196, 2500)
    ATOM_SET_THE_OFFSET_(197, 2501)
    ATOM_SET_THE_OFFSET_(198, 2502)
    ATOM_SET_THE_OFFSET_(199, 2503)
    ATOM_SET_THE_OFFSET_(200, 2504)
    ATOM_SET_THE_OFFSET_(201, 2505)
    ATOM_SET_THE_OFFSET_(202, 2506)
    ATOM_SET_THE_OFFSET_(203, 2507)
    ATOM_SET_THE_OFFSET_(204, 2508)
    ATOM_SET_THE_OFFSET_(205, 2509)
    ATOM_SET_THE_OFFSET_(206, 2510)
    ATOM_SET_THE_OFFSET_(207, 2511)
    ATOM_SET_THE_OFFSET_(208, 2512)
    ATOM_SET_THE_OFFSET_(209, 2513)
    ATOM_SET_THE_OFFSET_(210, 2514)
    ATOM_SET_THE_OFFSET_(211, 2515)
    ATOM_SET_THE_OFFSET_(212, 2516)
    ATOM_SET_THE_OFFSET_(213, 2517)
    ATOM_SET_THE_OFFSET_(214, 2518)
    ATOM_SET_THE_OFFSET_(215, 2519)
    ATOM_SET_THE_OFFSET_(216, 2520)
    ATOM_SET_THE_OFFSET_(217, 2521)
    ATOM_SET_THE_OFFSET_(218, 2522)
    ATOM_SET_THE_OFFSET_(219, 2523)
    ATOM_SET_THE_OFFSET_(220, 2524)
    ATOM_SET_THE_OFFSET_(221, 2525)
    ATOM_SET_THE_OFFSET_(222, 2526)
    ATOM_SET_THE_OFFSET_(223, 2527)
    ATOM_SET_THE_OFFSET_(224, 2528)
    ATOM_SET_THE_OFFSET_(225, 2529)
    ATOM_SET_THE_OFFSET_(226, 2530)
    ATOM_SET_THE_OFFSET_(227, 2531)
    ATOM_SET_THE_OFFSET_(228, 2532)
    ATOM_SET_THE_OFFSET_(229, 2533)
    ATOM_SET_THE_OFFSET_(230, 2534)
    ATOM_SET_THE_OFFSET_(231, 2535)
    ATOM_SET_THE_OFFSET_(232, 2536)
    ATOM_SET_THE_OFFSET_(233, 2537)
    ATOM_SET_THE_OFFSET_(234, 2538)
    ATOM_SET_THE_OFFSET_(235, 2539)
    ATOM_SET_THE_OFFSET_(236, 2540)
    ATOM_SET_THE_OFFSET_(237, 2541)
    ATOM_SET_THE_OFFSET_(238, 2542)
    ATOM_SET_THE_OFFSET_(239, 2543)
    ATOM_SET_THE_OFFSET_(240, 2544)
    ATOM_SET_THE_OFFSET_(241, 2545)
    ATOM_SET_THE_OFFSET_(242, 2546)
    ATOM_SET_THE_OFFSET_(243, 2547)
    ATOM_SET_THE_OFFSET_(244, 2548)
    ATOM_SET_THE_OFFSET_(245, 2549)
    ATOM_SET_THE_OFFSET_(246, 2550)
    ATOM_SET_THE_OFFSET_(247, 2551)
    ATOM_SET_THE_OFFSET_(248, 2552)
    ATOM_SET_THE_OFFSET_(249, 2553)
    ATOM_SET_THE_OFFSET_(250, 2554)
    ATOM_SET_THE_OFFSET_(251, 2555)
    ATOM_SET_THE_OFFSET_(252, 2556)
    ATOM_SET_THE_OFFSET_(253, 2557)
    ATOM_SET_THE_OFFSET_(254, 2558)
    ATOM_SET_THE_OFFSET_(255, 2559)
    return arr;
}

consteval static auto offset_map10(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 2560)
    ATOM_SET_THE_OFFSET_(1, 2561)
    ATOM_SET_THE_OFFSET_(2, 2562)
    ATOM_SET_THE_OFFSET_(3, 2563)
    ATOM_SET_THE_OFFSET_(4, 2564)
    ATOM_SET_THE_OFFSET_(5, 2565)
    ATOM_SET_THE_OFFSET_(6, 2566)
    ATOM_SET_THE_OFFSET_(7, 2567)
    ATOM_SET_THE_OFFSET_(8, 2568)
    ATOM_SET_THE_OFFSET_(9, 2569)
    ATOM_SET_THE_OFFSET_(10, 2570)
    ATOM_SET_THE_OFFSET_(11, 2571)
    ATOM_SET_THE_OFFSET_(12, 2572)
    ATOM_SET_THE_OFFSET_(13, 2573)
    ATOM_SET_THE_OFFSET_(14, 2574)
    ATOM_SET_THE_OFFSET_(15, 2575)
    ATOM_SET_THE_OFFSET_(16, 2576)
    ATOM_SET_THE_OFFSET_(17, 2577)
    ATOM_SET_THE_OFFSET_(18, 2578)
    ATOM_SET_THE_OFFSET_(19, 2579)
    ATOM_SET_THE_OFFSET_(20, 2580)
    ATOM_SET_THE_OFFSET_(21, 2581)
    ATOM_SET_THE_OFFSET_(22, 2582)
    ATOM_SET_THE_OFFSET_(23, 2583)
    ATOM_SET_THE_OFFSET_(24, 2584)
    ATOM_SET_THE_OFFSET_(25, 2585)
    ATOM_SET_THE_OFFSET_(26, 2586)
    ATOM_SET_THE_OFFSET_(27, 2587)
    ATOM_SET_THE_OFFSET_(28, 2588)
    ATOM_SET_THE_OFFSET_(29, 2589)
    ATOM_SET_THE_OFFSET_(30, 2590)
    ATOM_SET_THE_OFFSET_(31, 2591)
    ATOM_SET_THE_OFFSET_(32, 2592)
    ATOM_SET_THE_OFFSET_(33, 2593)
    ATOM_SET_THE_OFFSET_(34, 2594)
    ATOM_SET_THE_OFFSET_(35, 2595)
    ATOM_SET_THE_OFFSET_(36, 2596)
    ATOM_SET_THE_OFFSET_(37, 2597)
    ATOM_SET_THE_OFFSET_(38, 2598)
    ATOM_SET_THE_OFFSET_(39, 2599)
    ATOM_SET_THE_OFFSET_(40, 2600)
    ATOM_SET_THE_OFFSET_(41, 2601)
    ATOM_SET_THE_OFFSET_(42, 2602)
    ATOM_SET_THE_OFFSET_(43, 2603)
    ATOM_SET_THE_OFFSET_(44, 2604)
    ATOM_SET_THE_OFFSET_(45, 2605)
    ATOM_SET_THE_OFFSET_(46, 2606)
    ATOM_SET_THE_OFFSET_(47, 2607)
    ATOM_SET_THE_OFFSET_(48, 2608)
    ATOM_SET_THE_OFFSET_(49, 2609)
    ATOM_SET_THE_OFFSET_(50, 2610)
    ATOM_SET_THE_OFFSET_(51, 2611)
    ATOM_SET_THE_OFFSET_(52, 2612)
    ATOM_SET_THE_OFFSET_(53, 2613)
    ATOM_SET_THE_OFFSET_(54, 2614)
    ATOM_SET_THE_OFFSET_(55, 2615)
    ATOM_SET_THE_OFFSET_(56, 2616)
    ATOM_SET_THE_OFFSET_(57, 2617)
    ATOM_SET_THE_OFFSET_(58, 2618)
    ATOM_SET_THE_OFFSET_(59, 2619)
    ATOM_SET_THE_OFFSET_(60, 2620)
    ATOM_SET_THE_OFFSET_(61, 2621)
    ATOM_SET_THE_OFFSET_(62, 2622)
    ATOM_SET_THE_OFFSET_(63, 2623)
    ATOM_SET_THE_OFFSET_(64, 2624)
    ATOM_SET_THE_OFFSET_(65, 2625)
    ATOM_SET_THE_OFFSET_(66, 2626)
    ATOM_SET_THE_OFFSET_(67, 2627)
    ATOM_SET_THE_OFFSET_(68, 2628)
    ATOM_SET_THE_OFFSET_(69, 2629)
    ATOM_SET_THE_OFFSET_(70, 2630)
    ATOM_SET_THE_OFFSET_(71, 2631)
    ATOM_SET_THE_OFFSET_(72, 2632)
    ATOM_SET_THE_OFFSET_(73, 2633)
    ATOM_SET_THE_OFFSET_(74, 2634)
    ATOM_SET_THE_OFFSET_(75, 2635)
    ATOM_SET_THE_OFFSET_(76, 2636)
    ATOM_SET_THE_OFFSET_(77, 2637)
    ATOM_SET_THE_OFFSET_(78, 2638)
    ATOM_SET_THE_OFFSET_(79, 2639)
    ATOM_SET_THE_OFFSET_(80, 2640)
    ATOM_SET_THE_OFFSET_(81, 2641)
    ATOM_SET_THE_OFFSET_(82, 2642)
    ATOM_SET_THE_OFFSET_(83, 2643)
    ATOM_SET_THE_OFFSET_(84, 2644)
    ATOM_SET_THE_OFFSET_(85, 2645)
    ATOM_SET_THE_OFFSET_(86, 2646)
    ATOM_SET_THE_OFFSET_(87, 2647)
    ATOM_SET_THE_OFFSET_(88, 2648)
    ATOM_SET_THE_OFFSET_(89, 2649)
    ATOM_SET_THE_OFFSET_(90, 2650)
    ATOM_SET_THE_OFFSET_(91, 2651)
    ATOM_SET_THE_OFFSET_(92, 2652)
    ATOM_SET_THE_OFFSET_(93, 2653)
    ATOM_SET_THE_OFFSET_(94, 2654)
    ATOM_SET_THE_OFFSET_(95, 2655)
    ATOM_SET_THE_OFFSET_(96, 2656)
    ATOM_SET_THE_OFFSET_(97, 2657)
    ATOM_SET_THE_OFFSET_(98, 2658)
    ATOM_SET_THE_OFFSET_(99, 2659)
    ATOM_SET_THE_OFFSET_(100, 2660)
    ATOM_SET_THE_OFFSET_(101, 2661)
    ATOM_SET_THE_OFFSET_(102, 2662)
    ATOM_SET_THE_OFFSET_(103, 2663)
    ATOM_SET_THE_OFFSET_(104, 2664)
    ATOM_SET_THE_OFFSET_(105, 2665)
    ATOM_SET_THE_OFFSET_(106, 2666)
    ATOM_SET_THE_OFFSET_(107, 2667)
    ATOM_SET_THE_OFFSET_(108, 2668)
    ATOM_SET_THE_OFFSET_(109, 2669)
    ATOM_SET_THE_OFFSET_(110, 2670)
    ATOM_SET_THE_OFFSET_(111, 2671)
    ATOM_SET_THE_OFFSET_(112, 2672)
    ATOM_SET_THE_OFFSET_(113, 2673)
    ATOM_SET_THE_OFFSET_(114, 2674)
    ATOM_SET_THE_OFFSET_(115, 2675)
    ATOM_SET_THE_OFFSET_(116, 2676)
    ATOM_SET_THE_OFFSET_(117, 2677)
    ATOM_SET_THE_OFFSET_(118, 2678)
    ATOM_SET_THE_OFFSET_(119, 2679)
    ATOM_SET_THE_OFFSET_(120, 2680)
    ATOM_SET_THE_OFFSET_(121, 2681)
    ATOM_SET_THE_OFFSET_(122, 2682)
    ATOM_SET_THE_OFFSET_(123, 2683)
    ATOM_SET_THE_OFFSET_(124, 2684)
    ATOM_SET_THE_OFFSET_(125, 2685)
    ATOM_SET_THE_OFFSET_(126, 2686)
    ATOM_SET_THE_OFFSET_(127, 2687)
    ATOM_SET_THE_OFFSET_(128, 2688)
    ATOM_SET_THE_OFFSET_(129, 2689)
    ATOM_SET_THE_OFFSET_(130, 2690)
    ATOM_SET_THE_OFFSET_(131, 2691)
    ATOM_SET_THE_OFFSET_(132, 2692)
    ATOM_SET_THE_OFFSET_(133, 2693)
    ATOM_SET_THE_OFFSET_(134, 2694)
    ATOM_SET_THE_OFFSET_(135, 2695)
    ATOM_SET_THE_OFFSET_(136, 2696)
    ATOM_SET_THE_OFFSET_(137, 2697)
    ATOM_SET_THE_OFFSET_(138, 2698)
    ATOM_SET_THE_OFFSET_(139, 2699)
    ATOM_SET_THE_OFFSET_(140, 2700)
    ATOM_SET_THE_OFFSET_(141, 2701)
    ATOM_SET_THE_OFFSET_(142, 2702)
    ATOM_SET_THE_OFFSET_(143, 2703)
    ATOM_SET_THE_OFFSET_(144, 2704)
    ATOM_SET_THE_OFFSET_(145, 2705)
    ATOM_SET_THE_OFFSET_(146, 2706)
    ATOM_SET_THE_OFFSET_(147, 2707)
    ATOM_SET_THE_OFFSET_(148, 2708)
    ATOM_SET_THE_OFFSET_(149, 2709)
    ATOM_SET_THE_OFFSET_(150, 2710)
    ATOM_SET_THE_OFFSET_(151, 2711)
    ATOM_SET_THE_OFFSET_(152, 2712)
    ATOM_SET_THE_OFFSET_(153, 2713)
    ATOM_SET_THE_OFFSET_(154, 2714)
    ATOM_SET_THE_OFFSET_(155, 2715)
    ATOM_SET_THE_OFFSET_(156, 2716)
    ATOM_SET_THE_OFFSET_(157, 2717)
    ATOM_SET_THE_OFFSET_(158, 2718)
    ATOM_SET_THE_OFFSET_(159, 2719)
    ATOM_SET_THE_OFFSET_(160, 2720)
    ATOM_SET_THE_OFFSET_(161, 2721)
    ATOM_SET_THE_OFFSET_(162, 2722)
    ATOM_SET_THE_OFFSET_(163, 2723)
    ATOM_SET_THE_OFFSET_(164, 2724)
    ATOM_SET_THE_OFFSET_(165, 2725)
    ATOM_SET_THE_OFFSET_(166, 2726)
    ATOM_SET_THE_OFFSET_(167, 2727)
    ATOM_SET_THE_OFFSET_(168, 2728)
    ATOM_SET_THE_OFFSET_(169, 2729)
    ATOM_SET_THE_OFFSET_(170, 2730)
    ATOM_SET_THE_OFFSET_(171, 2731)
    ATOM_SET_THE_OFFSET_(172, 2732)
    ATOM_SET_THE_OFFSET_(173, 2733)
    ATOM_SET_THE_OFFSET_(174, 2734)
    ATOM_SET_THE_OFFSET_(175, 2735)
    ATOM_SET_THE_OFFSET_(176, 2736)
    ATOM_SET_THE_OFFSET_(177, 2737)
    ATOM_SET_THE_OFFSET_(178, 2738)
    ATOM_SET_THE_OFFSET_(179, 2739)
    ATOM_SET_THE_OFFSET_(180, 2740)
    ATOM_SET_THE_OFFSET_(181, 2741)
    ATOM_SET_THE_OFFSET_(182, 2742)
    ATOM_SET_THE_OFFSET_(183, 2743)
    ATOM_SET_THE_OFFSET_(184, 2744)
    ATOM_SET_THE_OFFSET_(185, 2745)
    ATOM_SET_THE_OFFSET_(186, 2746)
    ATOM_SET_THE_OFFSET_(187, 2747)
    ATOM_SET_THE_OFFSET_(188, 2748)
    ATOM_SET_THE_OFFSET_(189, 2749)
    ATOM_SET_THE_OFFSET_(190, 2750)
    ATOM_SET_THE_OFFSET_(191, 2751)
    ATOM_SET_THE_OFFSET_(192, 2752)
    ATOM_SET_THE_OFFSET_(193, 2753)
    ATOM_SET_THE_OFFSET_(194, 2754)
    ATOM_SET_THE_OFFSET_(195, 2755)
    ATOM_SET_THE_OFFSET_(196, 2756)
    ATOM_SET_THE_OFFSET_(197, 2757)
    ATOM_SET_THE_OFFSET_(198, 2758)
    ATOM_SET_THE_OFFSET_(199, 2759)
    ATOM_SET_THE_OFFSET_(200, 2760)
    ATOM_SET_THE_OFFSET_(201, 2761)
    ATOM_SET_THE_OFFSET_(202, 2762)
    ATOM_SET_THE_OFFSET_(203, 2763)
    ATOM_SET_THE_OFFSET_(204, 2764)
    ATOM_SET_THE_OFFSET_(205, 2765)
    ATOM_SET_THE_OFFSET_(206, 2766)
    ATOM_SET_THE_OFFSET_(207, 2767)
    ATOM_SET_THE_OFFSET_(208, 2768)
    ATOM_SET_THE_OFFSET_(209, 2769)
    ATOM_SET_THE_OFFSET_(210, 2770)
    ATOM_SET_THE_OFFSET_(211, 2771)
    ATOM_SET_THE_OFFSET_(212, 2772)
    ATOM_SET_THE_OFFSET_(213, 2773)
    ATOM_SET_THE_OFFSET_(214, 2774)
    ATOM_SET_THE_OFFSET_(215, 2775)
    ATOM_SET_THE_OFFSET_(216, 2776)
    ATOM_SET_THE_OFFSET_(217, 2777)
    ATOM_SET_THE_OFFSET_(218, 2778)
    ATOM_SET_THE_OFFSET_(219, 2779)
    ATOM_SET_THE_OFFSET_(220, 2780)
    ATOM_SET_THE_OFFSET_(221, 2781)
    ATOM_SET_THE_OFFSET_(222, 2782)
    ATOM_SET_THE_OFFSET_(223, 2783)
    ATOM_SET_THE_OFFSET_(224, 2784)
    ATOM_SET_THE_OFFSET_(225, 2785)
    ATOM_SET_THE_OFFSET_(226, 2786)
    ATOM_SET_THE_OFFSET_(227, 2787)
    ATOM_SET_THE_OFFSET_(228, 2788)
    ATOM_SET_THE_OFFSET_(229, 2789)
    ATOM_SET_THE_OFFSET_(230, 2790)
    ATOM_SET_THE_OFFSET_(231, 2791)
    ATOM_SET_THE_OFFSET_(232, 2792)
    ATOM_SET_THE_OFFSET_(233, 2793)
    ATOM_SET_THE_OFFSET_(234, 2794)
    ATOM_SET_THE_OFFSET_(235, 2795)
    ATOM_SET_THE_OFFSET_(236, 2796)
    ATOM_SET_THE_OFFSET_(237, 2797)
    ATOM_SET_THE_OFFSET_(238, 2798)
    ATOM_SET_THE_OFFSET_(239, 2799)
    ATOM_SET_THE_OFFSET_(240, 2800)
    ATOM_SET_THE_OFFSET_(241, 2801)
    ATOM_SET_THE_OFFSET_(242, 2802)
    ATOM_SET_THE_OFFSET_(243, 2803)
    ATOM_SET_THE_OFFSET_(244, 2804)
    ATOM_SET_THE_OFFSET_(245, 2805)
    ATOM_SET_THE_OFFSET_(246, 2806)
    ATOM_SET_THE_OFFSET_(247, 2807)
    ATOM_SET_THE_OFFSET_(248, 2808)
    ATOM_SET_THE_OFFSET_(249, 2809)
    ATOM_SET_THE_OFFSET_(250, 2810)
    ATOM_SET_THE_OFFSET_(251, 2811)
    ATOM_SET_THE_OFFSET_(252, 2812)
    ATOM_SET_THE_OFFSET_(253, 2813)
    ATOM_SET_THE_OFFSET_(254, 2814)
    ATOM_SET_THE_OFFSET_(255, 2815)
    return arr;
}

consteval static auto offset_map11(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 2816)
    ATOM_SET_THE_OFFSET_(1, 2817)
    ATOM_SET_THE_OFFSET_(2, 2818)
    ATOM_SET_THE_OFFSET_(3, 2819)
    ATOM_SET_THE_OFFSET_(4, 2820)
    ATOM_SET_THE_OFFSET_(5, 2821)
    ATOM_SET_THE_OFFSET_(6, 2822)
    ATOM_SET_THE_OFFSET_(7, 2823)
    ATOM_SET_THE_OFFSET_(8, 2824)
    ATOM_SET_THE_OFFSET_(9, 2825)
    ATOM_SET_THE_OFFSET_(10, 2826)
    ATOM_SET_THE_OFFSET_(11, 2827)
    ATOM_SET_THE_OFFSET_(12, 2828)
    ATOM_SET_THE_OFFSET_(13, 2829)
    ATOM_SET_THE_OFFSET_(14, 2830)
    ATOM_SET_THE_OFFSET_(15, 2831)
    ATOM_SET_THE_OFFSET_(16, 2832)
    ATOM_SET_THE_OFFSET_(17, 2833)
    ATOM_SET_THE_OFFSET_(18, 2834)
    ATOM_SET_THE_OFFSET_(19, 2835)
    ATOM_SET_THE_OFFSET_(20, 2836)
    ATOM_SET_THE_OFFSET_(21, 2837)
    ATOM_SET_THE_OFFSET_(22, 2838)
    ATOM_SET_THE_OFFSET_(23, 2839)
    ATOM_SET_THE_OFFSET_(24, 2840)
    ATOM_SET_THE_OFFSET_(25, 2841)
    ATOM_SET_THE_OFFSET_(26, 2842)
    ATOM_SET_THE_OFFSET_(27, 2843)
    ATOM_SET_THE_OFFSET_(28, 2844)
    ATOM_SET_THE_OFFSET_(29, 2845)
    ATOM_SET_THE_OFFSET_(30, 2846)
    ATOM_SET_THE_OFFSET_(31, 2847)
    ATOM_SET_THE_OFFSET_(32, 2848)
    ATOM_SET_THE_OFFSET_(33, 2849)
    ATOM_SET_THE_OFFSET_(34, 2850)
    ATOM_SET_THE_OFFSET_(35, 2851)
    ATOM_SET_THE_OFFSET_(36, 2852)
    ATOM_SET_THE_OFFSET_(37, 2853)
    ATOM_SET_THE_OFFSET_(38, 2854)
    ATOM_SET_THE_OFFSET_(39, 2855)
    ATOM_SET_THE_OFFSET_(40, 2856)
    ATOM_SET_THE_OFFSET_(41, 2857)
    ATOM_SET_THE_OFFSET_(42, 2858)
    ATOM_SET_THE_OFFSET_(43, 2859)
    ATOM_SET_THE_OFFSET_(44, 2860)
    ATOM_SET_THE_OFFSET_(45, 2861)
    ATOM_SET_THE_OFFSET_(46, 2862)
    ATOM_SET_THE_OFFSET_(47, 2863)
    ATOM_SET_THE_OFFSET_(48, 2864)
    ATOM_SET_THE_OFFSET_(49, 2865)
    ATOM_SET_THE_OFFSET_(50, 2866)
    ATOM_SET_THE_OFFSET_(51, 2867)
    ATOM_SET_THE_OFFSET_(52, 2868)
    ATOM_SET_THE_OFFSET_(53, 2869)
    ATOM_SET_THE_OFFSET_(54, 2870)
    ATOM_SET_THE_OFFSET_(55, 2871)
    ATOM_SET_THE_OFFSET_(56, 2872)
    ATOM_SET_THE_OFFSET_(57, 2873)
    ATOM_SET_THE_OFFSET_(58, 2874)
    ATOM_SET_THE_OFFSET_(59, 2875)
    ATOM_SET_THE_OFFSET_(60, 2876)
    ATOM_SET_THE_OFFSET_(61, 2877)
    ATOM_SET_THE_OFFSET_(62, 2878)
    ATOM_SET_THE_OFFSET_(63, 2879)
    ATOM_SET_THE_OFFSET_(64, 2880)
    ATOM_SET_THE_OFFSET_(65, 2881)
    ATOM_SET_THE_OFFSET_(66, 2882)
    ATOM_SET_THE_OFFSET_(67, 2883)
    ATOM_SET_THE_OFFSET_(68, 2884)
    ATOM_SET_THE_OFFSET_(69, 2885)
    ATOM_SET_THE_OFFSET_(70, 2886)
    ATOM_SET_THE_OFFSET_(71, 2887)
    ATOM_SET_THE_OFFSET_(72, 2888)
    ATOM_SET_THE_OFFSET_(73, 2889)
    ATOM_SET_THE_OFFSET_(74, 2890)
    ATOM_SET_THE_OFFSET_(75, 2891)
    ATOM_SET_THE_OFFSET_(76, 2892)
    ATOM_SET_THE_OFFSET_(77, 2893)
    ATOM_SET_THE_OFFSET_(78, 2894)
    ATOM_SET_THE_OFFSET_(79, 2895)
    ATOM_SET_THE_OFFSET_(80, 2896)
    ATOM_SET_THE_OFFSET_(81, 2897)
    ATOM_SET_THE_OFFSET_(82, 2898)
    ATOM_SET_THE_OFFSET_(83, 2899)
    ATOM_SET_THE_OFFSET_(84, 2900)
    ATOM_SET_THE_OFFSET_(85, 2901)
    ATOM_SET_THE_OFFSET_(86, 2902)
    ATOM_SET_THE_OFFSET_(87, 2903)
    ATOM_SET_THE_OFFSET_(88, 2904)
    ATOM_SET_THE_OFFSET_(89, 2905)
    ATOM_SET_THE_OFFSET_(90, 2906)
    ATOM_SET_THE_OFFSET_(91, 2907)
    ATOM_SET_THE_OFFSET_(92, 2908)
    ATOM_SET_THE_OFFSET_(93, 2909)
    ATOM_SET_THE_OFFSET_(94, 2910)
    ATOM_SET_THE_OFFSET_(95, 2911)
    ATOM_SET_THE_OFFSET_(96, 2912)
    ATOM_SET_THE_OFFSET_(97, 2913)
    ATOM_SET_THE_OFFSET_(98, 2914)
    ATOM_SET_THE_OFFSET_(99, 2915)
    ATOM_SET_THE_OFFSET_(100, 2916)
    ATOM_SET_THE_OFFSET_(101, 2917)
    ATOM_SET_THE_OFFSET_(102, 2918)
    ATOM_SET_THE_OFFSET_(103, 2919)
    ATOM_SET_THE_OFFSET_(104, 2920)
    ATOM_SET_THE_OFFSET_(105, 2921)
    ATOM_SET_THE_OFFSET_(106, 2922)
    ATOM_SET_THE_OFFSET_(107, 2923)
    ATOM_SET_THE_OFFSET_(108, 2924)
    ATOM_SET_THE_OFFSET_(109, 2925)
    ATOM_SET_THE_OFFSET_(110, 2926)
    ATOM_SET_THE_OFFSET_(111, 2927)
    ATOM_SET_THE_OFFSET_(112, 2928)
    ATOM_SET_THE_OFFSET_(113, 2929)
    ATOM_SET_THE_OFFSET_(114, 2930)
    ATOM_SET_THE_OFFSET_(115, 2931)
    ATOM_SET_THE_OFFSET_(116, 2932)
    ATOM_SET_THE_OFFSET_(117, 2933)
    ATOM_SET_THE_OFFSET_(118, 2934)
    ATOM_SET_THE_OFFSET_(119, 2935)
    ATOM_SET_THE_OFFSET_(120, 2936)
    ATOM_SET_THE_OFFSET_(121, 2937)
    ATOM_SET_THE_OFFSET_(122, 2938)
    ATOM_SET_THE_OFFSET_(123, 2939)
    ATOM_SET_THE_OFFSET_(124, 2940)
    ATOM_SET_THE_OFFSET_(125, 2941)
    ATOM_SET_THE_OFFSET_(126, 2942)
    ATOM_SET_THE_OFFSET_(127, 2943)
    ATOM_SET_THE_OFFSET_(128, 2944)
    ATOM_SET_THE_OFFSET_(129, 2945)
    ATOM_SET_THE_OFFSET_(130, 2946)
    ATOM_SET_THE_OFFSET_(131, 2947)
    ATOM_SET_THE_OFFSET_(132, 2948)
    ATOM_SET_THE_OFFSET_(133, 2949)
    ATOM_SET_THE_OFFSET_(134, 2950)
    ATOM_SET_THE_OFFSET_(135, 2951)
    ATOM_SET_THE_OFFSET_(136, 2952)
    ATOM_SET_THE_OFFSET_(137, 2953)
    ATOM_SET_THE_OFFSET_(138, 2954)
    ATOM_SET_THE_OFFSET_(139, 2955)
    ATOM_SET_THE_OFFSET_(140, 2956)
    ATOM_SET_THE_OFFSET_(141, 2957)
    ATOM_SET_THE_OFFSET_(142, 2958)
    ATOM_SET_THE_OFFSET_(143, 2959)
    ATOM_SET_THE_OFFSET_(144, 2960)
    ATOM_SET_THE_OFFSET_(145, 2961)
    ATOM_SET_THE_OFFSET_(146, 2962)
    ATOM_SET_THE_OFFSET_(147, 2963)
    ATOM_SET_THE_OFFSET_(148, 2964)
    ATOM_SET_THE_OFFSET_(149, 2965)
    ATOM_SET_THE_OFFSET_(150, 2966)
    ATOM_SET_THE_OFFSET_(151, 2967)
    ATOM_SET_THE_OFFSET_(152, 2968)
    ATOM_SET_THE_OFFSET_(153, 2969)
    ATOM_SET_THE_OFFSET_(154, 2970)
    ATOM_SET_THE_OFFSET_(155, 2971)
    ATOM_SET_THE_OFFSET_(156, 2972)
    ATOM_SET_THE_OFFSET_(157, 2973)
    ATOM_SET_THE_OFFSET_(158, 2974)
    ATOM_SET_THE_OFFSET_(159, 2975)
    ATOM_SET_THE_OFFSET_(160, 2976)
    ATOM_SET_THE_OFFSET_(161, 2977)
    ATOM_SET_THE_OFFSET_(162, 2978)
    ATOM_SET_THE_OFFSET_(163, 2979)
    ATOM_SET_THE_OFFSET_(164, 2980)
    ATOM_SET_THE_OFFSET_(165, 2981)
    ATOM_SET_THE_OFFSET_(166, 2982)
    ATOM_SET_THE_OFFSET_(167, 2983)
    ATOM_SET_THE_OFFSET_(168, 2984)
    ATOM_SET_THE_OFFSET_(169, 2985)
    ATOM_SET_THE_OFFSET_(170, 2986)
    ATOM_SET_THE_OFFSET_(171, 2987)
    ATOM_SET_THE_OFFSET_(172, 2988)
    ATOM_SET_THE_OFFSET_(173, 2989)
    ATOM_SET_THE_OFFSET_(174, 2990)
    ATOM_SET_THE_OFFSET_(175, 2991)
    ATOM_SET_THE_OFFSET_(176, 2992)
    ATOM_SET_THE_OFFSET_(177, 2993)
    ATOM_SET_THE_OFFSET_(178, 2994)
    ATOM_SET_THE_OFFSET_(179, 2995)
    ATOM_SET_THE_OFFSET_(180, 2996)
    ATOM_SET_THE_OFFSET_(181, 2997)
    ATOM_SET_THE_OFFSET_(182, 2998)
    ATOM_SET_THE_OFFSET_(183, 2999)
    ATOM_SET_THE_OFFSET_(184, 3000)
    ATOM_SET_THE_OFFSET_(185, 3001)
    ATOM_SET_THE_OFFSET_(186, 3002)
    ATOM_SET_THE_OFFSET_(187, 3003)
    ATOM_SET_THE_OFFSET_(188, 3004)
    ATOM_SET_THE_OFFSET_(189, 3005)
    ATOM_SET_THE_OFFSET_(190, 3006)
    ATOM_SET_THE_OFFSET_(191, 3007)
    ATOM_SET_THE_OFFSET_(192, 3008)
    ATOM_SET_THE_OFFSET_(193, 3009)
    ATOM_SET_THE_OFFSET_(194, 3010)
    ATOM_SET_THE_OFFSET_(195, 3011)
    ATOM_SET_THE_OFFSET_(196, 3012)
    ATOM_SET_THE_OFFSET_(197, 3013)
    ATOM_SET_THE_OFFSET_(198, 3014)
    ATOM_SET_THE_OFFSET_(199, 3015)
    ATOM_SET_THE_OFFSET_(200, 3016)
    ATOM_SET_THE_OFFSET_(201, 3017)
    ATOM_SET_THE_OFFSET_(202, 3018)
    ATOM_SET_THE_OFFSET_(203, 3019)
    ATOM_SET_THE_OFFSET_(204, 3020)
    ATOM_SET_THE_OFFSET_(205, 3021)
    ATOM_SET_THE_OFFSET_(206, 3022)
    ATOM_SET_THE_OFFSET_(207, 3023)
    ATOM_SET_THE_OFFSET_(208, 3024)
    ATOM_SET_THE_OFFSET_(209, 3025)
    ATOM_SET_THE_OFFSET_(210, 3026)
    ATOM_SET_THE_OFFSET_(211, 3027)
    ATOM_SET_THE_OFFSET_(212, 3028)
    ATOM_SET_THE_OFFSET_(213, 3029)
    ATOM_SET_THE_OFFSET_(214, 3030)
    ATOM_SET_THE_OFFSET_(215, 3031)
    ATOM_SET_THE_OFFSET_(216, 3032)
    ATOM_SET_THE_OFFSET_(217, 3033)
    ATOM_SET_THE_OFFSET_(218, 3034)
    ATOM_SET_THE_OFFSET_(219, 3035)
    ATOM_SET_THE_OFFSET_(220, 3036)
    ATOM_SET_THE_OFFSET_(221, 3037)
    ATOM_SET_THE_OFFSET_(222, 3038)
    ATOM_SET_THE_OFFSET_(223, 3039)
    ATOM_SET_THE_OFFSET_(224, 3040)
    ATOM_SET_THE_OFFSET_(225, 3041)
    ATOM_SET_THE_OFFSET_(226, 3042)
    ATOM_SET_THE_OFFSET_(227, 3043)
    ATOM_SET_THE_OFFSET_(228, 3044)
    ATOM_SET_THE_OFFSET_(229, 3045)
    ATOM_SET_THE_OFFSET_(230, 3046)
    ATOM_SET_THE_OFFSET_(231, 3047)
    ATOM_SET_THE_OFFSET_(232, 3048)
    ATOM_SET_THE_OFFSET_(233, 3049)
    ATOM_SET_THE_OFFSET_(234, 3050)
    ATOM_SET_THE_OFFSET_(235, 3051)
    ATOM_SET_THE_OFFSET_(236, 3052)
    ATOM_SET_THE_OFFSET_(237, 3053)
    ATOM_SET_THE_OFFSET_(238, 3054)
    ATOM_SET_THE_OFFSET_(239, 3055)
    ATOM_SET_THE_OFFSET_(240, 3056)
    ATOM_SET_THE_OFFSET_(241, 3057)
    ATOM_SET_THE_OFFSET_(242, 3058)
    ATOM_SET_THE_OFFSET_(243, 3059)
    ATOM_SET_THE_OFFSET_(244, 3060)
    ATOM_SET_THE_OFFSET_(245, 3061)
    ATOM_SET_THE_OFFSET_(246, 3062)
    ATOM_SET_THE_OFFSET_(247, 3063)
    ATOM_SET_THE_OFFSET_(248, 3064)
    ATOM_SET_THE_OFFSET_(249, 3065)
    ATOM_SET_THE_OFFSET_(250, 3066)
    ATOM_SET_THE_OFFSET_(251, 3067)
    ATOM_SET_THE_OFFSET_(252, 3068)
    ATOM_SET_THE_OFFSET_(253, 3069)
    ATOM_SET_THE_OFFSET_(254, 3070)
    ATOM_SET_THE_OFFSET_(255, 3071)
    return arr;
}

consteval static auto offset_map12(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 3072)
    ATOM_SET_THE_OFFSET_(1, 3073)
    ATOM_SET_THE_OFFSET_(2, 3074)
    ATOM_SET_THE_OFFSET_(3, 3075)
    ATOM_SET_THE_OFFSET_(4, 3076)
    ATOM_SET_THE_OFFSET_(5, 3077)
    ATOM_SET_THE_OFFSET_(6, 3078)
    ATOM_SET_THE_OFFSET_(7, 3079)
    ATOM_SET_THE_OFFSET_(8, 3080)
    ATOM_SET_THE_OFFSET_(9, 3081)
    ATOM_SET_THE_OFFSET_(10, 3082)
    ATOM_SET_THE_OFFSET_(11, 3083)
    ATOM_SET_THE_OFFSET_(12, 3084)
    ATOM_SET_THE_OFFSET_(13, 3085)
    ATOM_SET_THE_OFFSET_(14, 3086)
    ATOM_SET_THE_OFFSET_(15, 3087)
    ATOM_SET_THE_OFFSET_(16, 3088)
    ATOM_SET_THE_OFFSET_(17, 3089)
    ATOM_SET_THE_OFFSET_(18, 3090)
    ATOM_SET_THE_OFFSET_(19, 3091)
    ATOM_SET_THE_OFFSET_(20, 3092)
    ATOM_SET_THE_OFFSET_(21, 3093)
    ATOM_SET_THE_OFFSET_(22, 3094)
    ATOM_SET_THE_OFFSET_(23, 3095)
    ATOM_SET_THE_OFFSET_(24, 3096)
    ATOM_SET_THE_OFFSET_(25, 3097)
    ATOM_SET_THE_OFFSET_(26, 3098)
    ATOM_SET_THE_OFFSET_(27, 3099)
    ATOM_SET_THE_OFFSET_(28, 3100)
    ATOM_SET_THE_OFFSET_(29, 3101)
    ATOM_SET_THE_OFFSET_(30, 3102)
    ATOM_SET_THE_OFFSET_(31, 3103)
    ATOM_SET_THE_OFFSET_(32, 3104)
    ATOM_SET_THE_OFFSET_(33, 3105)
    ATOM_SET_THE_OFFSET_(34, 3106)
    ATOM_SET_THE_OFFSET_(35, 3107)
    ATOM_SET_THE_OFFSET_(36, 3108)
    ATOM_SET_THE_OFFSET_(37, 3109)
    ATOM_SET_THE_OFFSET_(38, 3110)
    ATOM_SET_THE_OFFSET_(39, 3111)
    ATOM_SET_THE_OFFSET_(40, 3112)
    ATOM_SET_THE_OFFSET_(41, 3113)
    ATOM_SET_THE_OFFSET_(42, 3114)
    ATOM_SET_THE_OFFSET_(43, 3115)
    ATOM_SET_THE_OFFSET_(44, 3116)
    ATOM_SET_THE_OFFSET_(45, 3117)
    ATOM_SET_THE_OFFSET_(46, 3118)
    ATOM_SET_THE_OFFSET_(47, 3119)
    ATOM_SET_THE_OFFSET_(48, 3120)
    ATOM_SET_THE_OFFSET_(49, 3121)
    ATOM_SET_THE_OFFSET_(50, 3122)
    ATOM_SET_THE_OFFSET_(51, 3123)
    ATOM_SET_THE_OFFSET_(52, 3124)
    ATOM_SET_THE_OFFSET_(53, 3125)
    ATOM_SET_THE_OFFSET_(54, 3126)
    ATOM_SET_THE_OFFSET_(55, 3127)
    ATOM_SET_THE_OFFSET_(56, 3128)
    ATOM_SET_THE_OFFSET_(57, 3129)
    ATOM_SET_THE_OFFSET_(58, 3130)
    ATOM_SET_THE_OFFSET_(59, 3131)
    ATOM_SET_THE_OFFSET_(60, 3132)
    ATOM_SET_THE_OFFSET_(61, 3133)
    ATOM_SET_THE_OFFSET_(62, 3134)
    ATOM_SET_THE_OFFSET_(63, 3135)
    ATOM_SET_THE_OFFSET_(64, 3136)
    ATOM_SET_THE_OFFSET_(65, 3137)
    ATOM_SET_THE_OFFSET_(66, 3138)
    ATOM_SET_THE_OFFSET_(67, 3139)
    ATOM_SET_THE_OFFSET_(68, 3140)
    ATOM_SET_THE_OFFSET_(69, 3141)
    ATOM_SET_THE_OFFSET_(70, 3142)
    ATOM_SET_THE_OFFSET_(71, 3143)
    ATOM_SET_THE_OFFSET_(72, 3144)
    ATOM_SET_THE_OFFSET_(73, 3145)
    ATOM_SET_THE_OFFSET_(74, 3146)
    ATOM_SET_THE_OFFSET_(75, 3147)
    ATOM_SET_THE_OFFSET_(76, 3148)
    ATOM_SET_THE_OFFSET_(77, 3149)
    ATOM_SET_THE_OFFSET_(78, 3150)
    ATOM_SET_THE_OFFSET_(79, 3151)
    ATOM_SET_THE_OFFSET_(80, 3152)
    ATOM_SET_THE_OFFSET_(81, 3153)
    ATOM_SET_THE_OFFSET_(82, 3154)
    ATOM_SET_THE_OFFSET_(83, 3155)
    ATOM_SET_THE_OFFSET_(84, 3156)
    ATOM_SET_THE_OFFSET_(85, 3157)
    ATOM_SET_THE_OFFSET_(86, 3158)
    ATOM_SET_THE_OFFSET_(87, 3159)
    ATOM_SET_THE_OFFSET_(88, 3160)
    ATOM_SET_THE_OFFSET_(89, 3161)
    ATOM_SET_THE_OFFSET_(90, 3162)
    ATOM_SET_THE_OFFSET_(91, 3163)
    ATOM_SET_THE_OFFSET_(92, 3164)
    ATOM_SET_THE_OFFSET_(93, 3165)
    ATOM_SET_THE_OFFSET_(94, 3166)
    ATOM_SET_THE_OFFSET_(95, 3167)
    ATOM_SET_THE_OFFSET_(96, 3168)
    ATOM_SET_THE_OFFSET_(97, 3169)
    ATOM_SET_THE_OFFSET_(98, 3170)
    ATOM_SET_THE_OFFSET_(99, 3171)
    ATOM_SET_THE_OFFSET_(100, 3172)
    ATOM_SET_THE_OFFSET_(101, 3173)
    ATOM_SET_THE_OFFSET_(102, 3174)
    ATOM_SET_THE_OFFSET_(103, 3175)
    ATOM_SET_THE_OFFSET_(104, 3176)
    ATOM_SET_THE_OFFSET_(105, 3177)
    ATOM_SET_THE_OFFSET_(106, 3178)
    ATOM_SET_THE_OFFSET_(107, 3179)
    ATOM_SET_THE_OFFSET_(108, 3180)
    ATOM_SET_THE_OFFSET_(109, 3181)
    ATOM_SET_THE_OFFSET_(110, 3182)
    ATOM_SET_THE_OFFSET_(111, 3183)
    ATOM_SET_THE_OFFSET_(112, 3184)
    ATOM_SET_THE_OFFSET_(113, 3185)
    ATOM_SET_THE_OFFSET_(114, 3186)
    ATOM_SET_THE_OFFSET_(115, 3187)
    ATOM_SET_THE_OFFSET_(116, 3188)
    ATOM_SET_THE_OFFSET_(117, 3189)
    ATOM_SET_THE_OFFSET_(118, 3190)
    ATOM_SET_THE_OFFSET_(119, 3191)
    ATOM_SET_THE_OFFSET_(120, 3192)
    ATOM_SET_THE_OFFSET_(121, 3193)
    ATOM_SET_THE_OFFSET_(122, 3194)
    ATOM_SET_THE_OFFSET_(123, 3195)
    ATOM_SET_THE_OFFSET_(124, 3196)
    ATOM_SET_THE_OFFSET_(125, 3197)
    ATOM_SET_THE_OFFSET_(126, 3198)
    ATOM_SET_THE_OFFSET_(127, 3199)
    ATOM_SET_THE_OFFSET_(128, 3200)
    ATOM_SET_THE_OFFSET_(129, 3201)
    ATOM_SET_THE_OFFSET_(130, 3202)
    ATOM_SET_THE_OFFSET_(131, 3203)
    ATOM_SET_THE_OFFSET_(132, 3204)
    ATOM_SET_THE_OFFSET_(133, 3205)
    ATOM_SET_THE_OFFSET_(134, 3206)
    ATOM_SET_THE_OFFSET_(135, 3207)
    ATOM_SET_THE_OFFSET_(136, 3208)
    ATOM_SET_THE_OFFSET_(137, 3209)
    ATOM_SET_THE_OFFSET_(138, 3210)
    ATOM_SET_THE_OFFSET_(139, 3211)
    ATOM_SET_THE_OFFSET_(140, 3212)
    ATOM_SET_THE_OFFSET_(141, 3213)
    ATOM_SET_THE_OFFSET_(142, 3214)
    ATOM_SET_THE_OFFSET_(143, 3215)
    ATOM_SET_THE_OFFSET_(144, 3216)
    ATOM_SET_THE_OFFSET_(145, 3217)
    ATOM_SET_THE_OFFSET_(146, 3218)
    ATOM_SET_THE_OFFSET_(147, 3219)
    ATOM_SET_THE_OFFSET_(148, 3220)
    ATOM_SET_THE_OFFSET_(149, 3221)
    ATOM_SET_THE_OFFSET_(150, 3222)
    ATOM_SET_THE_OFFSET_(151, 3223)
    ATOM_SET_THE_OFFSET_(152, 3224)
    ATOM_SET_THE_OFFSET_(153, 3225)
    ATOM_SET_THE_OFFSET_(154, 3226)
    ATOM_SET_THE_OFFSET_(155, 3227)
    ATOM_SET_THE_OFFSET_(156, 3228)
    ATOM_SET_THE_OFFSET_(157, 3229)
    ATOM_SET_THE_OFFSET_(158, 3230)
    ATOM_SET_THE_OFFSET_(159, 3231)
    ATOM_SET_THE_OFFSET_(160, 3232)
    ATOM_SET_THE_OFFSET_(161, 3233)
    ATOM_SET_THE_OFFSET_(162, 3234)
    ATOM_SET_THE_OFFSET_(163, 3235)
    ATOM_SET_THE_OFFSET_(164, 3236)
    ATOM_SET_THE_OFFSET_(165, 3237)
    ATOM_SET_THE_OFFSET_(166, 3238)
    ATOM_SET_THE_OFFSET_(167, 3239)
    ATOM_SET_THE_OFFSET_(168, 3240)
    ATOM_SET_THE_OFFSET_(169, 3241)
    ATOM_SET_THE_OFFSET_(170, 3242)
    ATOM_SET_THE_OFFSET_(171, 3243)
    ATOM_SET_THE_OFFSET_(172, 3244)
    ATOM_SET_THE_OFFSET_(173, 3245)
    ATOM_SET_THE_OFFSET_(174, 3246)
    ATOM_SET_THE_OFFSET_(175, 3247)
    ATOM_SET_THE_OFFSET_(176, 3248)
    ATOM_SET_THE_OFFSET_(177, 3249)
    ATOM_SET_THE_OFFSET_(178, 3250)
    ATOM_SET_THE_OFFSET_(179, 3251)
    ATOM_SET_THE_OFFSET_(180, 3252)
    ATOM_SET_THE_OFFSET_(181, 3253)
    ATOM_SET_THE_OFFSET_(182, 3254)
    ATOM_SET_THE_OFFSET_(183, 3255)
    ATOM_SET_THE_OFFSET_(184, 3256)
    ATOM_SET_THE_OFFSET_(185, 3257)
    ATOM_SET_THE_OFFSET_(186, 3258)
    ATOM_SET_THE_OFFSET_(187, 3259)
    ATOM_SET_THE_OFFSET_(188, 3260)
    ATOM_SET_THE_OFFSET_(189, 3261)
    ATOM_SET_THE_OFFSET_(190, 3262)
    ATOM_SET_THE_OFFSET_(191, 3263)
    ATOM_SET_THE_OFFSET_(192, 3264)
    ATOM_SET_THE_OFFSET_(193, 3265)
    ATOM_SET_THE_OFFSET_(194, 3266)
    ATOM_SET_THE_OFFSET_(195, 3267)
    ATOM_SET_THE_OFFSET_(196, 3268)
    ATOM_SET_THE_OFFSET_(197, 3269)
    ATOM_SET_THE_OFFSET_(198, 3270)
    ATOM_SET_THE_OFFSET_(199, 3271)
    ATOM_SET_THE_OFFSET_(200, 3272)
    ATOM_SET_THE_OFFSET_(201, 3273)
    ATOM_SET_THE_OFFSET_(202, 3274)
    ATOM_SET_THE_OFFSET_(203, 3275)
    ATOM_SET_THE_OFFSET_(204, 3276)
    ATOM_SET_THE_OFFSET_(205, 3277)
    ATOM_SET_THE_OFFSET_(206, 3278)
    ATOM_SET_THE_OFFSET_(207, 3279)
    ATOM_SET_THE_OFFSET_(208, 3280)
    ATOM_SET_THE_OFFSET_(209, 3281)
    ATOM_SET_THE_OFFSET_(210, 3282)
    ATOM_SET_THE_OFFSET_(211, 3283)
    ATOM_SET_THE_OFFSET_(212, 3284)
    ATOM_SET_THE_OFFSET_(213, 3285)
    ATOM_SET_THE_OFFSET_(214, 3286)
    ATOM_SET_THE_OFFSET_(215, 3287)
    ATOM_SET_THE_OFFSET_(216, 3288)
    ATOM_SET_THE_OFFSET_(217, 3289)
    ATOM_SET_THE_OFFSET_(218, 3290)
    ATOM_SET_THE_OFFSET_(219, 3291)
    ATOM_SET_THE_OFFSET_(220, 3292)
    ATOM_SET_THE_OFFSET_(221, 3293)
    ATOM_SET_THE_OFFSET_(222, 3294)
    ATOM_SET_THE_OFFSET_(223, 3295)
    ATOM_SET_THE_OFFSET_(224, 3296)
    ATOM_SET_THE_OFFSET_(225, 3297)
    ATOM_SET_THE_OFFSET_(226, 3298)
    ATOM_SET_THE_OFFSET_(227, 3299)
    ATOM_SET_THE_OFFSET_(228, 3300)
    ATOM_SET_THE_OFFSET_(229, 3301)
    ATOM_SET_THE_OFFSET_(230, 3302)
    ATOM_SET_THE_OFFSET_(231, 3303)
    ATOM_SET_THE_OFFSET_(232, 3304)
    ATOM_SET_THE_OFFSET_(233, 3305)
    ATOM_SET_THE_OFFSET_(234, 3306)
    ATOM_SET_THE_OFFSET_(235, 3307)
    ATOM_SET_THE_OFFSET_(236, 3308)
    ATOM_SET_THE_OFFSET_(237, 3309)
    ATOM_SET_THE_OFFSET_(238, 3310)
    ATOM_SET_THE_OFFSET_(239, 3311)
    ATOM_SET_THE_OFFSET_(240, 3312)
    ATOM_SET_THE_OFFSET_(241, 3313)
    ATOM_SET_THE_OFFSET_(242, 3314)
    ATOM_SET_THE_OFFSET_(243, 3315)
    ATOM_SET_THE_OFFSET_(244, 3316)
    ATOM_SET_THE_OFFSET_(245, 3317)
    ATOM_SET_THE_OFFSET_(246, 3318)
    ATOM_SET_THE_OFFSET_(247, 3319)
    ATOM_SET_THE_OFFSET_(248, 3320)
    ATOM_SET_THE_OFFSET_(249, 3321)
    ATOM_SET_THE_OFFSET_(250, 3322)
    ATOM_SET_THE_OFFSET_(251, 3323)
    ATOM_SET_THE_OFFSET_(252, 3324)
    ATOM_SET_THE_OFFSET_(253, 3325)
    ATOM_SET_THE_OFFSET_(254, 3326)
    ATOM_SET_THE_OFFSET_(255, 3327)
    return arr;
}

consteval static auto offset_map13(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 3328)
    ATOM_SET_THE_OFFSET_(1, 3329)
    ATOM_SET_THE_OFFSET_(2, 3330)
    ATOM_SET_THE_OFFSET_(3, 3331)
    ATOM_SET_THE_OFFSET_(4, 3332)
    ATOM_SET_THE_OFFSET_(5, 3333)
    ATOM_SET_THE_OFFSET_(6, 3334)
    ATOM_SET_THE_OFFSET_(7, 3335)
    ATOM_SET_THE_OFFSET_(8, 3336)
    ATOM_SET_THE_OFFSET_(9, 3337)
    ATOM_SET_THE_OFFSET_(10, 3338)
    ATOM_SET_THE_OFFSET_(11, 3339)
    ATOM_SET_THE_OFFSET_(12, 3340)
    ATOM_SET_THE_OFFSET_(13, 3341)
    ATOM_SET_THE_OFFSET_(14, 3342)
    ATOM_SET_THE_OFFSET_(15, 3343)
    ATOM_SET_THE_OFFSET_(16, 3344)
    ATOM_SET_THE_OFFSET_(17, 3345)
    ATOM_SET_THE_OFFSET_(18, 3346)
    ATOM_SET_THE_OFFSET_(19, 3347)
    ATOM_SET_THE_OFFSET_(20, 3348)
    ATOM_SET_THE_OFFSET_(21, 3349)
    ATOM_SET_THE_OFFSET_(22, 3350)
    ATOM_SET_THE_OFFSET_(23, 3351)
    ATOM_SET_THE_OFFSET_(24, 3352)
    ATOM_SET_THE_OFFSET_(25, 3353)
    ATOM_SET_THE_OFFSET_(26, 3354)
    ATOM_SET_THE_OFFSET_(27, 3355)
    ATOM_SET_THE_OFFSET_(28, 3356)
    ATOM_SET_THE_OFFSET_(29, 3357)
    ATOM_SET_THE_OFFSET_(30, 3358)
    ATOM_SET_THE_OFFSET_(31, 3359)
    ATOM_SET_THE_OFFSET_(32, 3360)
    ATOM_SET_THE_OFFSET_(33, 3361)
    ATOM_SET_THE_OFFSET_(34, 3362)
    ATOM_SET_THE_OFFSET_(35, 3363)
    ATOM_SET_THE_OFFSET_(36, 3364)
    ATOM_SET_THE_OFFSET_(37, 3365)
    ATOM_SET_THE_OFFSET_(38, 3366)
    ATOM_SET_THE_OFFSET_(39, 3367)
    ATOM_SET_THE_OFFSET_(40, 3368)
    ATOM_SET_THE_OFFSET_(41, 3369)
    ATOM_SET_THE_OFFSET_(42, 3370)
    ATOM_SET_THE_OFFSET_(43, 3371)
    ATOM_SET_THE_OFFSET_(44, 3372)
    ATOM_SET_THE_OFFSET_(45, 3373)
    ATOM_SET_THE_OFFSET_(46, 3374)
    ATOM_SET_THE_OFFSET_(47, 3375)
    ATOM_SET_THE_OFFSET_(48, 3376)
    ATOM_SET_THE_OFFSET_(49, 3377)
    ATOM_SET_THE_OFFSET_(50, 3378)
    ATOM_SET_THE_OFFSET_(51, 3379)
    ATOM_SET_THE_OFFSET_(52, 3380)
    ATOM_SET_THE_OFFSET_(53, 3381)
    ATOM_SET_THE_OFFSET_(54, 3382)
    ATOM_SET_THE_OFFSET_(55, 3383)
    ATOM_SET_THE_OFFSET_(56, 3384)
    ATOM_SET_THE_OFFSET_(57, 3385)
    ATOM_SET_THE_OFFSET_(58, 3386)
    ATOM_SET_THE_OFFSET_(59, 3387)
    ATOM_SET_THE_OFFSET_(60, 3388)
    ATOM_SET_THE_OFFSET_(61, 3389)
    ATOM_SET_THE_OFFSET_(62, 3390)
    ATOM_SET_THE_OFFSET_(63, 3391)
    ATOM_SET_THE_OFFSET_(64, 3392)
    ATOM_SET_THE_OFFSET_(65, 3393)
    ATOM_SET_THE_OFFSET_(66, 3394)
    ATOM_SET_THE_OFFSET_(67, 3395)
    ATOM_SET_THE_OFFSET_(68, 3396)
    ATOM_SET_THE_OFFSET_(69, 3397)
    ATOM_SET_THE_OFFSET_(70, 3398)
    ATOM_SET_THE_OFFSET_(71, 3399)
    ATOM_SET_THE_OFFSET_(72, 3400)
    ATOM_SET_THE_OFFSET_(73, 3401)
    ATOM_SET_THE_OFFSET_(74, 3402)
    ATOM_SET_THE_OFFSET_(75, 3403)
    ATOM_SET_THE_OFFSET_(76, 3404)
    ATOM_SET_THE_OFFSET_(77, 3405)
    ATOM_SET_THE_OFFSET_(78, 3406)
    ATOM_SET_THE_OFFSET_(79, 3407)
    ATOM_SET_THE_OFFSET_(80, 3408)
    ATOM_SET_THE_OFFSET_(81, 3409)
    ATOM_SET_THE_OFFSET_(82, 3410)
    ATOM_SET_THE_OFFSET_(83, 3411)
    ATOM_SET_THE_OFFSET_(84, 3412)
    ATOM_SET_THE_OFFSET_(85, 3413)
    ATOM_SET_THE_OFFSET_(86, 3414)
    ATOM_SET_THE_OFFSET_(87, 3415)
    ATOM_SET_THE_OFFSET_(88, 3416)
    ATOM_SET_THE_OFFSET_(89, 3417)
    ATOM_SET_THE_OFFSET_(90, 3418)
    ATOM_SET_THE_OFFSET_(91, 3419)
    ATOM_SET_THE_OFFSET_(92, 3420)
    ATOM_SET_THE_OFFSET_(93, 3421)
    ATOM_SET_THE_OFFSET_(94, 3422)
    ATOM_SET_THE_OFFSET_(95, 3423)
    ATOM_SET_THE_OFFSET_(96, 3424)
    ATOM_SET_THE_OFFSET_(97, 3425)
    ATOM_SET_THE_OFFSET_(98, 3426)
    ATOM_SET_THE_OFFSET_(99, 3427)
    ATOM_SET_THE_OFFSET_(100, 3428)
    ATOM_SET_THE_OFFSET_(101, 3429)
    ATOM_SET_THE_OFFSET_(102, 3430)
    ATOM_SET_THE_OFFSET_(103, 3431)
    ATOM_SET_THE_OFFSET_(104, 3432)
    ATOM_SET_THE_OFFSET_(105, 3433)
    ATOM_SET_THE_OFFSET_(106, 3434)
    ATOM_SET_THE_OFFSET_(107, 3435)
    ATOM_SET_THE_OFFSET_(108, 3436)
    ATOM_SET_THE_OFFSET_(109, 3437)
    ATOM_SET_THE_OFFSET_(110, 3438)
    ATOM_SET_THE_OFFSET_(111, 3439)
    ATOM_SET_THE_OFFSET_(112, 3440)
    ATOM_SET_THE_OFFSET_(113, 3441)
    ATOM_SET_THE_OFFSET_(114, 3442)
    ATOM_SET_THE_OFFSET_(115, 3443)
    ATOM_SET_THE_OFFSET_(116, 3444)
    ATOM_SET_THE_OFFSET_(117, 3445)
    ATOM_SET_THE_OFFSET_(118, 3446)
    ATOM_SET_THE_OFFSET_(119, 3447)
    ATOM_SET_THE_OFFSET_(120, 3448)
    ATOM_SET_THE_OFFSET_(121, 3449)
    ATOM_SET_THE_OFFSET_(122, 3450)
    ATOM_SET_THE_OFFSET_(123, 3451)
    ATOM_SET_THE_OFFSET_(124, 3452)
    ATOM_SET_THE_OFFSET_(125, 3453)
    ATOM_SET_THE_OFFSET_(126, 3454)
    ATOM_SET_THE_OFFSET_(127, 3455)
    ATOM_SET_THE_OFFSET_(128, 3456)
    ATOM_SET_THE_OFFSET_(129, 3457)
    ATOM_SET_THE_OFFSET_(130, 3458)
    ATOM_SET_THE_OFFSET_(131, 3459)
    ATOM_SET_THE_OFFSET_(132, 3460)
    ATOM_SET_THE_OFFSET_(133, 3461)
    ATOM_SET_THE_OFFSET_(134, 3462)
    ATOM_SET_THE_OFFSET_(135, 3463)
    ATOM_SET_THE_OFFSET_(136, 3464)
    ATOM_SET_THE_OFFSET_(137, 3465)
    ATOM_SET_THE_OFFSET_(138, 3466)
    ATOM_SET_THE_OFFSET_(139, 3467)
    ATOM_SET_THE_OFFSET_(140, 3468)
    ATOM_SET_THE_OFFSET_(141, 3469)
    ATOM_SET_THE_OFFSET_(142, 3470)
    ATOM_SET_THE_OFFSET_(143, 3471)
    ATOM_SET_THE_OFFSET_(144, 3472)
    ATOM_SET_THE_OFFSET_(145, 3473)
    ATOM_SET_THE_OFFSET_(146, 3474)
    ATOM_SET_THE_OFFSET_(147, 3475)
    ATOM_SET_THE_OFFSET_(148, 3476)
    ATOM_SET_THE_OFFSET_(149, 3477)
    ATOM_SET_THE_OFFSET_(150, 3478)
    ATOM_SET_THE_OFFSET_(151, 3479)
    ATOM_SET_THE_OFFSET_(152, 3480)
    ATOM_SET_THE_OFFSET_(153, 3481)
    ATOM_SET_THE_OFFSET_(154, 3482)
    ATOM_SET_THE_OFFSET_(155, 3483)
    ATOM_SET_THE_OFFSET_(156, 3484)
    ATOM_SET_THE_OFFSET_(157, 3485)
    ATOM_SET_THE_OFFSET_(158, 3486)
    ATOM_SET_THE_OFFSET_(159, 3487)
    ATOM_SET_THE_OFFSET_(160, 3488)
    ATOM_SET_THE_OFFSET_(161, 3489)
    ATOM_SET_THE_OFFSET_(162, 3490)
    ATOM_SET_THE_OFFSET_(163, 3491)
    ATOM_SET_THE_OFFSET_(164, 3492)
    ATOM_SET_THE_OFFSET_(165, 3493)
    ATOM_SET_THE_OFFSET_(166, 3494)
    ATOM_SET_THE_OFFSET_(167, 3495)
    ATOM_SET_THE_OFFSET_(168, 3496)
    ATOM_SET_THE_OFFSET_(169, 3497)
    ATOM_SET_THE_OFFSET_(170, 3498)
    ATOM_SET_THE_OFFSET_(171, 3499)
    ATOM_SET_THE_OFFSET_(172, 3500)
    ATOM_SET_THE_OFFSET_(173, 3501)
    ATOM_SET_THE_OFFSET_(174, 3502)
    ATOM_SET_THE_OFFSET_(175, 3503)
    ATOM_SET_THE_OFFSET_(176, 3504)
    ATOM_SET_THE_OFFSET_(177, 3505)
    ATOM_SET_THE_OFFSET_(178, 3506)
    ATOM_SET_THE_OFFSET_(179, 3507)
    ATOM_SET_THE_OFFSET_(180, 3508)
    ATOM_SET_THE_OFFSET_(181, 3509)
    ATOM_SET_THE_OFFSET_(182, 3510)
    ATOM_SET_THE_OFFSET_(183, 3511)
    ATOM_SET_THE_OFFSET_(184, 3512)
    ATOM_SET_THE_OFFSET_(185, 3513)
    ATOM_SET_THE_OFFSET_(186, 3514)
    ATOM_SET_THE_OFFSET_(187, 3515)
    ATOM_SET_THE_OFFSET_(188, 3516)
    ATOM_SET_THE_OFFSET_(189, 3517)
    ATOM_SET_THE_OFFSET_(190, 3518)
    ATOM_SET_THE_OFFSET_(191, 3519)
    ATOM_SET_THE_OFFSET_(192, 3520)
    ATOM_SET_THE_OFFSET_(193, 3521)
    ATOM_SET_THE_OFFSET_(194, 3522)
    ATOM_SET_THE_OFFSET_(195, 3523)
    ATOM_SET_THE_OFFSET_(196, 3524)
    ATOM_SET_THE_OFFSET_(197, 3525)
    ATOM_SET_THE_OFFSET_(198, 3526)
    ATOM_SET_THE_OFFSET_(199, 3527)
    ATOM_SET_THE_OFFSET_(200, 3528)
    ATOM_SET_THE_OFFSET_(201, 3529)
    ATOM_SET_THE_OFFSET_(202, 3530)
    ATOM_SET_THE_OFFSET_(203, 3531)
    ATOM_SET_THE_OFFSET_(204, 3532)
    ATOM_SET_THE_OFFSET_(205, 3533)
    ATOM_SET_THE_OFFSET_(206, 3534)
    ATOM_SET_THE_OFFSET_(207, 3535)
    ATOM_SET_THE_OFFSET_(208, 3536)
    ATOM_SET_THE_OFFSET_(209, 3537)
    ATOM_SET_THE_OFFSET_(210, 3538)
    ATOM_SET_THE_OFFSET_(211, 3539)
    ATOM_SET_THE_OFFSET_(212, 3540)
    ATOM_SET_THE_OFFSET_(213, 3541)
    ATOM_SET_THE_OFFSET_(214, 3542)
    ATOM_SET_THE_OFFSET_(215, 3543)
    ATOM_SET_THE_OFFSET_(216, 3544)
    ATOM_SET_THE_OFFSET_(217, 3545)
    ATOM_SET_THE_OFFSET_(218, 3546)
    ATOM_SET_THE_OFFSET_(219, 3547)
    ATOM_SET_THE_OFFSET_(220, 3548)
    ATOM_SET_THE_OFFSET_(221, 3549)
    ATOM_SET_THE_OFFSET_(222, 3550)
    ATOM_SET_THE_OFFSET_(223, 3551)
    ATOM_SET_THE_OFFSET_(224, 3552)
    ATOM_SET_THE_OFFSET_(225, 3553)
    ATOM_SET_THE_OFFSET_(226, 3554)
    ATOM_SET_THE_OFFSET_(227, 3555)
    ATOM_SET_THE_OFFSET_(228, 3556)
    ATOM_SET_THE_OFFSET_(229, 3557)
    ATOM_SET_THE_OFFSET_(230, 3558)
    ATOM_SET_THE_OFFSET_(231, 3559)
    ATOM_SET_THE_OFFSET_(232, 3560)
    ATOM_SET_THE_OFFSET_(233, 3561)
    ATOM_SET_THE_OFFSET_(234, 3562)
    ATOM_SET_THE_OFFSET_(235, 3563)
    ATOM_SET_THE_OFFSET_(236, 3564)
    ATOM_SET_THE_OFFSET_(237, 3565)
    ATOM_SET_THE_OFFSET_(238, 3566)
    ATOM_SET_THE_OFFSET_(239, 3567)
    ATOM_SET_THE_OFFSET_(240, 3568)
    ATOM_SET_THE_OFFSET_(241, 3569)
    ATOM_SET_THE_OFFSET_(242, 3570)
    ATOM_SET_THE_OFFSET_(243, 3571)
    ATOM_SET_THE_OFFSET_(244, 3572)
    ATOM_SET_THE_OFFSET_(245, 3573)
    ATOM_SET_THE_OFFSET_(246, 3574)
    ATOM_SET_THE_OFFSET_(247, 3575)
    ATOM_SET_THE_OFFSET_(248, 3576)
    ATOM_SET_THE_OFFSET_(249, 3577)
    ATOM_SET_THE_OFFSET_(250, 3578)
    ATOM_SET_THE_OFFSET_(251, 3579)
    ATOM_SET_THE_OFFSET_(252, 3580)
    ATOM_SET_THE_OFFSET_(253, 3581)
    ATOM_SET_THE_OFFSET_(254, 3582)
    ATOM_SET_THE_OFFSET_(255, 3583)
    return arr;
}

consteval static auto offset_map14(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 3584)
    ATOM_SET_THE_OFFSET_(1, 3585)
    ATOM_SET_THE_OFFSET_(2, 3586)
    ATOM_SET_THE_OFFSET_(3, 3587)
    ATOM_SET_THE_OFFSET_(4, 3588)
    ATOM_SET_THE_OFFSET_(5, 3589)
    ATOM_SET_THE_OFFSET_(6, 3590)
    ATOM_SET_THE_OFFSET_(7, 3591)
    ATOM_SET_THE_OFFSET_(8, 3592)
    ATOM_SET_THE_OFFSET_(9, 3593)
    ATOM_SET_THE_OFFSET_(10, 3594)
    ATOM_SET_THE_OFFSET_(11, 3595)
    ATOM_SET_THE_OFFSET_(12, 3596)
    ATOM_SET_THE_OFFSET_(13, 3597)
    ATOM_SET_THE_OFFSET_(14, 3598)
    ATOM_SET_THE_OFFSET_(15, 3599)
    ATOM_SET_THE_OFFSET_(16, 3600)
    ATOM_SET_THE_OFFSET_(17, 3601)
    ATOM_SET_THE_OFFSET_(18, 3602)
    ATOM_SET_THE_OFFSET_(19, 3603)
    ATOM_SET_THE_OFFSET_(20, 3604)
    ATOM_SET_THE_OFFSET_(21, 3605)
    ATOM_SET_THE_OFFSET_(22, 3606)
    ATOM_SET_THE_OFFSET_(23, 3607)
    ATOM_SET_THE_OFFSET_(24, 3608)
    ATOM_SET_THE_OFFSET_(25, 3609)
    ATOM_SET_THE_OFFSET_(26, 3610)
    ATOM_SET_THE_OFFSET_(27, 3611)
    ATOM_SET_THE_OFFSET_(28, 3612)
    ATOM_SET_THE_OFFSET_(29, 3613)
    ATOM_SET_THE_OFFSET_(30, 3614)
    ATOM_SET_THE_OFFSET_(31, 3615)
    ATOM_SET_THE_OFFSET_(32, 3616)
    ATOM_SET_THE_OFFSET_(33, 3617)
    ATOM_SET_THE_OFFSET_(34, 3618)
    ATOM_SET_THE_OFFSET_(35, 3619)
    ATOM_SET_THE_OFFSET_(36, 3620)
    ATOM_SET_THE_OFFSET_(37, 3621)
    ATOM_SET_THE_OFFSET_(38, 3622)
    ATOM_SET_THE_OFFSET_(39, 3623)
    ATOM_SET_THE_OFFSET_(40, 3624)
    ATOM_SET_THE_OFFSET_(41, 3625)
    ATOM_SET_THE_OFFSET_(42, 3626)
    ATOM_SET_THE_OFFSET_(43, 3627)
    ATOM_SET_THE_OFFSET_(44, 3628)
    ATOM_SET_THE_OFFSET_(45, 3629)
    ATOM_SET_THE_OFFSET_(46, 3630)
    ATOM_SET_THE_OFFSET_(47, 3631)
    ATOM_SET_THE_OFFSET_(48, 3632)
    ATOM_SET_THE_OFFSET_(49, 3633)
    ATOM_SET_THE_OFFSET_(50, 3634)
    ATOM_SET_THE_OFFSET_(51, 3635)
    ATOM_SET_THE_OFFSET_(52, 3636)
    ATOM_SET_THE_OFFSET_(53, 3637)
    ATOM_SET_THE_OFFSET_(54, 3638)
    ATOM_SET_THE_OFFSET_(55, 3639)
    ATOM_SET_THE_OFFSET_(56, 3640)
    ATOM_SET_THE_OFFSET_(57, 3641)
    ATOM_SET_THE_OFFSET_(58, 3642)
    ATOM_SET_THE_OFFSET_(59, 3643)
    ATOM_SET_THE_OFFSET_(60, 3644)
    ATOM_SET_THE_OFFSET_(61, 3645)
    ATOM_SET_THE_OFFSET_(62, 3646)
    ATOM_SET_THE_OFFSET_(63, 3647)
    ATOM_SET_THE_OFFSET_(64, 3648)
    ATOM_SET_THE_OFFSET_(65, 3649)
    ATOM_SET_THE_OFFSET_(66, 3650)
    ATOM_SET_THE_OFFSET_(67, 3651)
    ATOM_SET_THE_OFFSET_(68, 3652)
    ATOM_SET_THE_OFFSET_(69, 3653)
    ATOM_SET_THE_OFFSET_(70, 3654)
    ATOM_SET_THE_OFFSET_(71, 3655)
    ATOM_SET_THE_OFFSET_(72, 3656)
    ATOM_SET_THE_OFFSET_(73, 3657)
    ATOM_SET_THE_OFFSET_(74, 3658)
    ATOM_SET_THE_OFFSET_(75, 3659)
    ATOM_SET_THE_OFFSET_(76, 3660)
    ATOM_SET_THE_OFFSET_(77, 3661)
    ATOM_SET_THE_OFFSET_(78, 3662)
    ATOM_SET_THE_OFFSET_(79, 3663)
    ATOM_SET_THE_OFFSET_(80, 3664)
    ATOM_SET_THE_OFFSET_(81, 3665)
    ATOM_SET_THE_OFFSET_(82, 3666)
    ATOM_SET_THE_OFFSET_(83, 3667)
    ATOM_SET_THE_OFFSET_(84, 3668)
    ATOM_SET_THE_OFFSET_(85, 3669)
    ATOM_SET_THE_OFFSET_(86, 3670)
    ATOM_SET_THE_OFFSET_(87, 3671)
    ATOM_SET_THE_OFFSET_(88, 3672)
    ATOM_SET_THE_OFFSET_(89, 3673)
    ATOM_SET_THE_OFFSET_(90, 3674)
    ATOM_SET_THE_OFFSET_(91, 3675)
    ATOM_SET_THE_OFFSET_(92, 3676)
    ATOM_SET_THE_OFFSET_(93, 3677)
    ATOM_SET_THE_OFFSET_(94, 3678)
    ATOM_SET_THE_OFFSET_(95, 3679)
    ATOM_SET_THE_OFFSET_(96, 3680)
    ATOM_SET_THE_OFFSET_(97, 3681)
    ATOM_SET_THE_OFFSET_(98, 3682)
    ATOM_SET_THE_OFFSET_(99, 3683)
    ATOM_SET_THE_OFFSET_(100, 3684)
    ATOM_SET_THE_OFFSET_(101, 3685)
    ATOM_SET_THE_OFFSET_(102, 3686)
    ATOM_SET_THE_OFFSET_(103, 3687)
    ATOM_SET_THE_OFFSET_(104, 3688)
    ATOM_SET_THE_OFFSET_(105, 3689)
    ATOM_SET_THE_OFFSET_(106, 3690)
    ATOM_SET_THE_OFFSET_(107, 3691)
    ATOM_SET_THE_OFFSET_(108, 3692)
    ATOM_SET_THE_OFFSET_(109, 3693)
    ATOM_SET_THE_OFFSET_(110, 3694)
    ATOM_SET_THE_OFFSET_(111, 3695)
    ATOM_SET_THE_OFFSET_(112, 3696)
    ATOM_SET_THE_OFFSET_(113, 3697)
    ATOM_SET_THE_OFFSET_(114, 3698)
    ATOM_SET_THE_OFFSET_(115, 3699)
    ATOM_SET_THE_OFFSET_(116, 3700)
    ATOM_SET_THE_OFFSET_(117, 3701)
    ATOM_SET_THE_OFFSET_(118, 3702)
    ATOM_SET_THE_OFFSET_(119, 3703)
    ATOM_SET_THE_OFFSET_(120, 3704)
    ATOM_SET_THE_OFFSET_(121, 3705)
    ATOM_SET_THE_OFFSET_(122, 3706)
    ATOM_SET_THE_OFFSET_(123, 3707)
    ATOM_SET_THE_OFFSET_(124, 3708)
    ATOM_SET_THE_OFFSET_(125, 3709)
    ATOM_SET_THE_OFFSET_(126, 3710)
    ATOM_SET_THE_OFFSET_(127, 3711)
    ATOM_SET_THE_OFFSET_(128, 3712)
    ATOM_SET_THE_OFFSET_(129, 3713)
    ATOM_SET_THE_OFFSET_(130, 3714)
    ATOM_SET_THE_OFFSET_(131, 3715)
    ATOM_SET_THE_OFFSET_(132, 3716)
    ATOM_SET_THE_OFFSET_(133, 3717)
    ATOM_SET_THE_OFFSET_(134, 3718)
    ATOM_SET_THE_OFFSET_(135, 3719)
    ATOM_SET_THE_OFFSET_(136, 3720)
    ATOM_SET_THE_OFFSET_(137, 3721)
    ATOM_SET_THE_OFFSET_(138, 3722)
    ATOM_SET_THE_OFFSET_(139, 3723)
    ATOM_SET_THE_OFFSET_(140, 3724)
    ATOM_SET_THE_OFFSET_(141, 3725)
    ATOM_SET_THE_OFFSET_(142, 3726)
    ATOM_SET_THE_OFFSET_(143, 3727)
    ATOM_SET_THE_OFFSET_(144, 3728)
    ATOM_SET_THE_OFFSET_(145, 3729)
    ATOM_SET_THE_OFFSET_(146, 3730)
    ATOM_SET_THE_OFFSET_(147, 3731)
    ATOM_SET_THE_OFFSET_(148, 3732)
    ATOM_SET_THE_OFFSET_(149, 3733)
    ATOM_SET_THE_OFFSET_(150, 3734)
    ATOM_SET_THE_OFFSET_(151, 3735)
    ATOM_SET_THE_OFFSET_(152, 3736)
    ATOM_SET_THE_OFFSET_(153, 3737)
    ATOM_SET_THE_OFFSET_(154, 3738)
    ATOM_SET_THE_OFFSET_(155, 3739)
    ATOM_SET_THE_OFFSET_(156, 3740)
    ATOM_SET_THE_OFFSET_(157, 3741)
    ATOM_SET_THE_OFFSET_(158, 3742)
    ATOM_SET_THE_OFFSET_(159, 3743)
    ATOM_SET_THE_OFFSET_(160, 3744)
    ATOM_SET_THE_OFFSET_(161, 3745)
    ATOM_SET_THE_OFFSET_(162, 3746)
    ATOM_SET_THE_OFFSET_(163, 3747)
    ATOM_SET_THE_OFFSET_(164, 3748)
    ATOM_SET_THE_OFFSET_(165, 3749)
    ATOM_SET_THE_OFFSET_(166, 3750)
    ATOM_SET_THE_OFFSET_(167, 3751)
    ATOM_SET_THE_OFFSET_(168, 3752)
    ATOM_SET_THE_OFFSET_(169, 3753)
    ATOM_SET_THE_OFFSET_(170, 3754)
    ATOM_SET_THE_OFFSET_(171, 3755)
    ATOM_SET_THE_OFFSET_(172, 3756)
    ATOM_SET_THE_OFFSET_(173, 3757)
    ATOM_SET_THE_OFFSET_(174, 3758)
    ATOM_SET_THE_OFFSET_(175, 3759)
    ATOM_SET_THE_OFFSET_(176, 3760)
    ATOM_SET_THE_OFFSET_(177, 3761)
    ATOM_SET_THE_OFFSET_(178, 3762)
    ATOM_SET_THE_OFFSET_(179, 3763)
    ATOM_SET_THE_OFFSET_(180, 3764)
    ATOM_SET_THE_OFFSET_(181, 3765)
    ATOM_SET_THE_OFFSET_(182, 3766)
    ATOM_SET_THE_OFFSET_(183, 3767)
    ATOM_SET_THE_OFFSET_(184, 3768)
    ATOM_SET_THE_OFFSET_(185, 3769)
    ATOM_SET_THE_OFFSET_(186, 3770)
    ATOM_SET_THE_OFFSET_(187, 3771)
    ATOM_SET_THE_OFFSET_(188, 3772)
    ATOM_SET_THE_OFFSET_(189, 3773)
    ATOM_SET_THE_OFFSET_(190, 3774)
    ATOM_SET_THE_OFFSET_(191, 3775)
    ATOM_SET_THE_OFFSET_(192, 3776)
    ATOM_SET_THE_OFFSET_(193, 3777)
    ATOM_SET_THE_OFFSET_(194, 3778)
    ATOM_SET_THE_OFFSET_(195, 3779)
    ATOM_SET_THE_OFFSET_(196, 3780)
    ATOM_SET_THE_OFFSET_(197, 3781)
    ATOM_SET_THE_OFFSET_(198, 3782)
    ATOM_SET_THE_OFFSET_(199, 3783)
    ATOM_SET_THE_OFFSET_(200, 3784)
    ATOM_SET_THE_OFFSET_(201, 3785)
    ATOM_SET_THE_OFFSET_(202, 3786)
    ATOM_SET_THE_OFFSET_(203, 3787)
    ATOM_SET_THE_OFFSET_(204, 3788)
    ATOM_SET_THE_OFFSET_(205, 3789)
    ATOM_SET_THE_OFFSET_(206, 3790)
    ATOM_SET_THE_OFFSET_(207, 3791)
    ATOM_SET_THE_OFFSET_(208, 3792)
    ATOM_SET_THE_OFFSET_(209, 3793)
    ATOM_SET_THE_OFFSET_(210, 3794)
    ATOM_SET_THE_OFFSET_(211, 3795)
    ATOM_SET_THE_OFFSET_(212, 3796)
    ATOM_SET_THE_OFFSET_(213, 3797)
    ATOM_SET_THE_OFFSET_(214, 3798)
    ATOM_SET_THE_OFFSET_(215, 3799)
    ATOM_SET_THE_OFFSET_(216, 3800)
    ATOM_SET_THE_OFFSET_(217, 3801)
    ATOM_SET_THE_OFFSET_(218, 3802)
    ATOM_SET_THE_OFFSET_(219, 3803)
    ATOM_SET_THE_OFFSET_(220, 3804)
    ATOM_SET_THE_OFFSET_(221, 3805)
    ATOM_SET_THE_OFFSET_(222, 3806)
    ATOM_SET_THE_OFFSET_(223, 3807)
    ATOM_SET_THE_OFFSET_(224, 3808)
    ATOM_SET_THE_OFFSET_(225, 3809)
    ATOM_SET_THE_OFFSET_(226, 3810)
    ATOM_SET_THE_OFFSET_(227, 3811)
    ATOM_SET_THE_OFFSET_(228, 3812)
    ATOM_SET_THE_OFFSET_(229, 3813)
    ATOM_SET_THE_OFFSET_(230, 3814)
    ATOM_SET_THE_OFFSET_(231, 3815)
    ATOM_SET_THE_OFFSET_(232, 3816)
    ATOM_SET_THE_OFFSET_(233, 3817)
    ATOM_SET_THE_OFFSET_(234, 3818)
    ATOM_SET_THE_OFFSET_(235, 3819)
    ATOM_SET_THE_OFFSET_(236, 3820)
    ATOM_SET_THE_OFFSET_(237, 3821)
    ATOM_SET_THE_OFFSET_(238, 3822)
    ATOM_SET_THE_OFFSET_(239, 3823)
    ATOM_SET_THE_OFFSET_(240, 3824)
    ATOM_SET_THE_OFFSET_(241, 3825)
    ATOM_SET_THE_OFFSET_(242, 3826)
    ATOM_SET_THE_OFFSET_(243, 3827)
    ATOM_SET_THE_OFFSET_(244, 3828)
    ATOM_SET_THE_OFFSET_(245, 3829)
    ATOM_SET_THE_OFFSET_(246, 3830)
    ATOM_SET_THE_OFFSET_(247, 3831)
    ATOM_SET_THE_OFFSET_(248, 3832)
    ATOM_SET_THE_OFFSET_(249, 3833)
    ATOM_SET_THE_OFFSET_(250, 3834)
    ATOM_SET_THE_OFFSET_(251, 3835)
    ATOM_SET_THE_OFFSET_(252, 3836)
    ATOM_SET_THE_OFFSET_(253, 3837)
    ATOM_SET_THE_OFFSET_(254, 3838)
    ATOM_SET_THE_OFFSET_(255, 3839)
    return arr;
}

consteval static auto offset_map15(const std::size_t off) noexcept {
    std::array<std::uint8_t offset_helper::*, offset_map_size> arr{};
    ATOM_SET_THE_OFFSET_(0, 3840)
    ATOM_SET_THE_OFFSET_(1, 3841)
    ATOM_SET_THE_OFFSET_(2, 3842)
    ATOM_SET_THE_OFFSET_(3, 3843)
    ATOM_SET_THE_OFFSET_(4, 3844)
    ATOM_SET_THE_OFFSET_(5, 3845)
    ATOM_SET_THE_OFFSET_(6, 3846)
    ATOM_SET_THE_OFFSET_(7, 3847)
    ATOM_SET_THE_OFFSET_(8, 3848)
    ATOM_SET_THE_OFFSET_(9, 3849)
    ATOM_SET_THE_OFFSET_(10, 3850)
    ATOM_SET_THE_OFFSET_(11, 3851)
    ATOM_SET_THE_OFFSET_(12, 3852)
    ATOM_SET_THE_OFFSET_(13, 3853)
    ATOM_SET_THE_OFFSET_(14, 3854)
    ATOM_SET_THE_OFFSET_(15, 3855)
    ATOM_SET_THE_OFFSET_(16, 3856)
    ATOM_SET_THE_OFFSET_(17, 3857)
    ATOM_SET_THE_OFFSET_(18, 3858)
    ATOM_SET_THE_OFFSET_(19, 3859)
    ATOM_SET_THE_OFFSET_(20, 3860)
    ATOM_SET_THE_OFFSET_(21, 3861)
    ATOM_SET_THE_OFFSET_(22, 3862)
    ATOM_SET_THE_OFFSET_(23, 3863)
    ATOM_SET_THE_OFFSET_(24, 3864)
    ATOM_SET_THE_OFFSET_(25, 3865)
    ATOM_SET_THE_OFFSET_(26, 3866)
    ATOM_SET_THE_OFFSET_(27, 3867)
    ATOM_SET_THE_OFFSET_(28, 3868)
    ATOM_SET_THE_OFFSET_(29, 3869)
    ATOM_SET_THE_OFFSET_(30, 3870)
    ATOM_SET_THE_OFFSET_(31, 3871)
    ATOM_SET_THE_OFFSET_(32, 3872)
    ATOM_SET_THE_OFFSET_(33, 3873)
    ATOM_SET_THE_OFFSET_(34, 3874)
    ATOM_SET_THE_OFFSET_(35, 3875)
    ATOM_SET_THE_OFFSET_(36, 3876)
    ATOM_SET_THE_OFFSET_(37, 3877)
    ATOM_SET_THE_OFFSET_(38, 3878)
    ATOM_SET_THE_OFFSET_(39, 3879)
    ATOM_SET_THE_OFFSET_(40, 3880)
    ATOM_SET_THE_OFFSET_(41, 3881)
    ATOM_SET_THE_OFFSET_(42, 3882)
    ATOM_SET_THE_OFFSET_(43, 3883)
    ATOM_SET_THE_OFFSET_(44, 3884)
    ATOM_SET_THE_OFFSET_(45, 3885)
    ATOM_SET_THE_OFFSET_(46, 3886)
    ATOM_SET_THE_OFFSET_(47, 3887)
    ATOM_SET_THE_OFFSET_(48, 3888)
    ATOM_SET_THE_OFFSET_(49, 3889)
    ATOM_SET_THE_OFFSET_(50, 3890)
    ATOM_SET_THE_OFFSET_(51, 3891)
    ATOM_SET_THE_OFFSET_(52, 3892)
    ATOM_SET_THE_OFFSET_(53, 3893)
    ATOM_SET_THE_OFFSET_(54, 3894)
    ATOM_SET_THE_OFFSET_(55, 3895)
    ATOM_SET_THE_OFFSET_(56, 3896)
    ATOM_SET_THE_OFFSET_(57, 3897)
    ATOM_SET_THE_OFFSET_(58, 3898)
    ATOM_SET_THE_OFFSET_(59, 3899)
    ATOM_SET_THE_OFFSET_(60, 3900)
    ATOM_SET_THE_OFFSET_(61, 3901)
    ATOM_SET_THE_OFFSET_(62, 3902)
    ATOM_SET_THE_OFFSET_(63, 3903)
    ATOM_SET_THE_OFFSET_(64, 3904)
    ATOM_SET_THE_OFFSET_(65, 3905)
    ATOM_SET_THE_OFFSET_(66, 3906)
    ATOM_SET_THE_OFFSET_(67, 3907)
    ATOM_SET_THE_OFFSET_(68, 3908)
    ATOM_SET_THE_OFFSET_(69, 3909)
    ATOM_SET_THE_OFFSET_(70, 3910)
    ATOM_SET_THE_OFFSET_(71, 3911)
    ATOM_SET_THE_OFFSET_(72, 3912)
    ATOM_SET_THE_OFFSET_(73, 3913)
    ATOM_SET_THE_OFFSET_(74, 3914)
    ATOM_SET_THE_OFFSET_(75, 3915)
    ATOM_SET_THE_OFFSET_(76, 3916)
    ATOM_SET_THE_OFFSET_(77, 3917)
    ATOM_SET_THE_OFFSET_(78, 3918)
    ATOM_SET_THE_OFFSET_(79, 3919)
    ATOM_SET_THE_OFFSET_(80, 3920)
    ATOM_SET_THE_OFFSET_(81, 3921)
    ATOM_SET_THE_OFFSET_(82, 3922)
    ATOM_SET_THE_OFFSET_(83, 3923)
    ATOM_SET_THE_OFFSET_(84, 3924)
    ATOM_SET_THE_OFFSET_(85, 3925)
    ATOM_SET_THE_OFFSET_(86, 3926)
    ATOM_SET_THE_OFFSET_(87, 3927)
    ATOM_SET_THE_OFFSET_(88, 3928)
    ATOM_SET_THE_OFFSET_(89, 3929)
    ATOM_SET_THE_OFFSET_(90, 3930)
    ATOM_SET_THE_OFFSET_(91, 3931)
    ATOM_SET_THE_OFFSET_(92, 3932)
    ATOM_SET_THE_OFFSET_(93, 3933)
    ATOM_SET_THE_OFFSET_(94, 3934)
    ATOM_SET_THE_OFFSET_(95, 3935)
    ATOM_SET_THE_OFFSET_(96, 3936)
    ATOM_SET_THE_OFFSET_(97, 3937)
    ATOM_SET_THE_OFFSET_(98, 3938)
    ATOM_SET_THE_OFFSET_(99, 3939)
    ATOM_SET_THE_OFFSET_(100, 3940)
    ATOM_SET_THE_OFFSET_(101, 3941)
    ATOM_SET_THE_OFFSET_(102, 3942)
    ATOM_SET_THE_OFFSET_(103, 3943)
    ATOM_SET_THE_OFFSET_(104, 3944)
    ATOM_SET_THE_OFFSET_(105, 3945)
    ATOM_SET_THE_OFFSET_(106, 3946)
    ATOM_SET_THE_OFFSET_(107, 3947)
    ATOM_SET_THE_OFFSET_(108, 3948)
    ATOM_SET_THE_OFFSET_(109, 3949)
    ATOM_SET_THE_OFFSET_(110, 3950)
    ATOM_SET_THE_OFFSET_(111, 3951)
    ATOM_SET_THE_OFFSET_(112, 3952)
    ATOM_SET_THE_OFFSET_(113, 3953)
    ATOM_SET_THE_OFFSET_(114, 3954)
    ATOM_SET_THE_OFFSET_(115, 3955)
    ATOM_SET_THE_OFFSET_(116, 3956)
    ATOM_SET_THE_OFFSET_(117, 3957)
    ATOM_SET_THE_OFFSET_(118, 3958)
    ATOM_SET_THE_OFFSET_(119, 3959)
    ATOM_SET_THE_OFFSET_(120, 3960)
    ATOM_SET_THE_OFFSET_(121, 3961)
    ATOM_SET_THE_OFFSET_(122, 3962)
    ATOM_SET_THE_OFFSET_(123, 3963)
    ATOM_SET_THE_OFFSET_(124, 3964)
    ATOM_SET_THE_OFFSET_(125, 3965)
    ATOM_SET_THE_OFFSET_(126, 3966)
    ATOM_SET_THE_OFFSET_(127, 3967)
    ATOM_SET_THE_OFFSET_(128, 3968)
    ATOM_SET_THE_OFFSET_(129, 3969)
    ATOM_SET_THE_OFFSET_(130, 3970)
    ATOM_SET_THE_OFFSET_(131, 3971)
    ATOM_SET_THE_OFFSET_(132, 3972)
    ATOM_SET_THE_OFFSET_(133, 3973)
    ATOM_SET_THE_OFFSET_(134, 3974)
    ATOM_SET_THE_OFFSET_(135, 3975)
    ATOM_SET_THE_OFFSET_(136, 3976)
    ATOM_SET_THE_OFFSET_(137, 3977)
    ATOM_SET_THE_OFFSET_(138, 3978)
    ATOM_SET_THE_OFFSET_(139, 3979)
    ATOM_SET_THE_OFFSET_(140, 3980)
    ATOM_SET_THE_OFFSET_(141, 3981)
    ATOM_SET_THE_OFFSET_(142, 3982)
    ATOM_SET_THE_OFFSET_(143, 3983)
    ATOM_SET_THE_OFFSET_(144, 3984)
    ATOM_SET_THE_OFFSET_(145, 3985)
    ATOM_SET_THE_OFFSET_(146, 3986)
    ATOM_SET_THE_OFFSET_(147, 3987)
    ATOM_SET_THE_OFFSET_(148, 3988)
    ATOM_SET_THE_OFFSET_(149, 3989)
    ATOM_SET_THE_OFFSET_(150, 3990)
    ATOM_SET_THE_OFFSET_(151, 3991)
    ATOM_SET_THE_OFFSET_(152, 3992)
    ATOM_SET_THE_OFFSET_(153, 3993)
    ATOM_SET_THE_OFFSET_(154, 3994)
    ATOM_SET_THE_OFFSET_(155, 3995)
    ATOM_SET_THE_OFFSET_(156, 3996)
    ATOM_SET_THE_OFFSET_(157, 3997)
    ATOM_SET_THE_OFFSET_(158, 3998)
    ATOM_SET_THE_OFFSET_(159, 3999)
    ATOM_SET_THE_OFFSET_(160, 4000)
    ATOM_SET_THE_OFFSET_(161, 4001)
    ATOM_SET_THE_OFFSET_(162, 4002)
    ATOM_SET_THE_OFFSET_(163, 4003)
    ATOM_SET_THE_OFFSET_(164, 4004)
    ATOM_SET_THE_OFFSET_(165, 4005)
    ATOM_SET_THE_OFFSET_(166, 4006)
    ATOM_SET_THE_OFFSET_(167, 4007)
    ATOM_SET_THE_OFFSET_(168, 4008)
    ATOM_SET_THE_OFFSET_(169, 4009)
    ATOM_SET_THE_OFFSET_(170, 4010)
    ATOM_SET_THE_OFFSET_(171, 4011)
    ATOM_SET_THE_OFFSET_(172, 4012)
    ATOM_SET_THE_OFFSET_(173, 4013)
    ATOM_SET_THE_OFFSET_(174, 4014)
    ATOM_SET_THE_OFFSET_(175, 4015)
    ATOM_SET_THE_OFFSET_(176, 4016)
    ATOM_SET_THE_OFFSET_(177, 4017)
    ATOM_SET_THE_OFFSET_(178, 4018)
    ATOM_SET_THE_OFFSET_(179, 4019)
    ATOM_SET_THE_OFFSET_(180, 4020)
    ATOM_SET_THE_OFFSET_(181, 4021)
    ATOM_SET_THE_OFFSET_(182, 4022)
    ATOM_SET_THE_OFFSET_(183, 4023)
    ATOM_SET_THE_OFFSET_(184, 4024)
    ATOM_SET_THE_OFFSET_(185, 4025)
    ATOM_SET_THE_OFFSET_(186, 4026)
    ATOM_SET_THE_OFFSET_(187, 4027)
    ATOM_SET_THE_OFFSET_(188, 4028)
    ATOM_SET_THE_OFFSET_(189, 4029)
    ATOM_SET_THE_OFFSET_(190, 4030)
    ATOM_SET_THE_OFFSET_(191, 4031)
    ATOM_SET_THE_OFFSET_(192, 4032)
    ATOM_SET_THE_OFFSET_(193, 4033)
    ATOM_SET_THE_OFFSET_(194, 4034)
    ATOM_SET_THE_OFFSET_(195, 4035)
    ATOM_SET_THE_OFFSET_(196, 4036)
    ATOM_SET_THE_OFFSET_(197, 4037)
    ATOM_SET_THE_OFFSET_(198, 4038)
    ATOM_SET_THE_OFFSET_(199, 4039)
    ATOM_SET_THE_OFFSET_(200, 4040)
    ATOM_SET_THE_OFFSET_(201, 4041)
    ATOM_SET_THE_OFFSET_(202, 4042)
    ATOM_SET_THE_OFFSET_(203, 4043)
    ATOM_SET_THE_OFFSET_(204, 4044)
    ATOM_SET_THE_OFFSET_(205, 4045)
    ATOM_SET_THE_OFFSET_(206, 4046)
    ATOM_SET_THE_OFFSET_(207, 4047)
    ATOM_SET_THE_OFFSET_(208, 4048)
    ATOM_SET_THE_OFFSET_(209, 4049)
    ATOM_SET_THE_OFFSET_(210, 4050)
    ATOM_SET_THE_OFFSET_(211, 4051)
    ATOM_SET_THE_OFFSET_(212, 4052)
    ATOM_SET_THE_OFFSET_(213, 4053)
    ATOM_SET_THE_OFFSET_(214, 4054)
    ATOM_SET_THE_OFFSET_(215, 4055)
    ATOM_SET_THE_OFFSET_(216, 4056)
    ATOM_SET_THE_OFFSET_(217, 4057)
    ATOM_SET_THE_OFFSET_(218, 4058)
    ATOM_SET_THE_OFFSET_(219, 4059)
    ATOM_SET_THE_OFFSET_(220, 4060)
    ATOM_SET_THE_OFFSET_(221, 4061)
    ATOM_SET_THE_OFFSET_(222, 4062)
    ATOM_SET_THE_OFFSET_(223, 4063)
    ATOM_SET_THE_OFFSET_(224, 4064)
    ATOM_SET_THE_OFFSET_(225, 4065)
    ATOM_SET_THE_OFFSET_(226, 4066)
    ATOM_SET_THE_OFFSET_(227, 4067)
    ATOM_SET_THE_OFFSET_(228, 4068)
    ATOM_SET_THE_OFFSET_(229, 4069)
    ATOM_SET_THE_OFFSET_(230, 4070)
    ATOM_SET_THE_OFFSET_(231, 4071)
    ATOM_SET_THE_OFFSET_(232, 4072)
    ATOM_SET_THE_OFFSET_(233, 4073)
    ATOM_SET_THE_OFFSET_(234, 4074)
    ATOM_SET_THE_OFFSET_(235, 4075)
    ATOM_SET_THE_OFFSET_(236, 4076)
    ATOM_SET_THE_OFFSET_(237, 4077)
    ATOM_SET_THE_OFFSET_(238, 4078)
    ATOM_SET_THE_OFFSET_(239, 4079)
    ATOM_SET_THE_OFFSET_(240, 4080)
    ATOM_SET_THE_OFFSET_(241, 4081)
    ATOM_SET_THE_OFFSET_(242, 4082)
    ATOM_SET_THE_OFFSET_(243, 4083)
    ATOM_SET_THE_OFFSET_(244, 4084)
    ATOM_SET_THE_OFFSET_(245, 4085)
    ATOM_SET_THE_OFFSET_(246, 4086)
    ATOM_SET_THE_OFFSET_(247, 4087)
    ATOM_SET_THE_OFFSET_(248, 4088)
    ATOM_SET_THE_OFFSET_(249, 4089)
    ATOM_SET_THE_OFFSET_(250, 4090)
    ATOM_SET_THE_OFFSET_(251, 4091)
    ATOM_SET_THE_OFFSET_(252, 4092)
    ATOM_SET_THE_OFFSET_(253, 4093)
    ATOM_SET_THE_OFFSET_(254, 4094)
    ATOM_SET_THE_OFFSET_(255, 4095)
    return arr;
}

constexpr static auto offset_mapping0(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping1(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping2(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping3(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping4(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping5(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping6(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping7(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping8(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping9(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping10(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping11(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping12(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping13(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping14(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

constexpr static auto offset_mapping15(const std::size_t off) noexcept {
    constexpr std::array<std::uint8_t offset_helper::*, offset_map_size> offsets = offset_map0();
    return offsets.at(off);
}

} // namespace

constexpr static inline auto offset_mapping_(const std::size_t offset) noexcept {
    const std::size_t map = offset / offset_map_size;
    const std::size_t off = offset % offset_map_size;

    const auto c = 4096 / offset_map_size;

    // we believe that, in general, not too much storage is used, otherwise there is a design
    // problem.
    if (!map) [[likely]] { // 0
        return offset_mapping0(off);
    }
    else [[unlikely]] {           // 1 ~ 15
        if (map < 7) [[likely]] { // 1 ~ 7
            if (map < 4) [[likely]] {
                switch (map) {
                [[likely]] case 1:
                    return offset_mapping1(off);
                case 2:
                    return offset_mapping2(off);
                default:
                    return offset_mapping3(off);
                }
            }
            else { // 4 ~ 7
                switch (map) {
                [[likely]] case 4:
                    return offset_mapping4(off);
                case 5:
                    return offset_mapping5(off);
                case 6:
                    return offset_mapping6(off);
                default:
                    return offset_mapping7(off);
                }
            }
        }
        else {              // 8 ~ 15
            if (map < 12) { // 8 ~ 11
                switch (map) {
                [[likely]] case 8:
                    return offset_mapping8(off);
                case 9:
                    return offset_mapping9(off);
                case 10:
                    return offset_mapping10(off);
                [[unlikely]] default:
                    return offset_mapping11(off);
                }
            }
            else { // 12 ~ 15
                switch (map) {
                [[likely]] case 12:
                    return offset_mapping12(off);
                case 13:
                    return offset_mapping13(off);
                case 14:
                    return offset_mapping14(off);
                [[unlikely]] case 15:
                    return offset_mapping15(off);
                [[unlikely]] default:
                    return static_cast<std::uint8_t offset_helper::*>(nullptr);
                }
            }
        }
    }
}

#undef ATOM_CONCAT__
#undef ATOM_CONCAT_
#undef ATOM_SET_THE_OFFSET_
// NOLINTEND(cppcoreguidelines-macro-usage)

template <concepts::aggregate Ty>
static inline auto offset_tuple() noexcept {
    static_assert(
        sizeof(Ty) < offset_count, "The size is larger than the max offset could get "
                                   "automatically! Please get the offsets manually.");
    constexpr auto vw   = struct_to_tuple_view<Ty>();
    auto tuple          = make_offset_tuple<Ty>(vw);
    const auto& offsets = offsets_of<Ty>();

    // set offsets in tuple
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        ((std::get<Is>(tuple) =
              (std::tuple_element_t<Is, decltype(tuple)>)offset_mapping_(offsets[Is])),
         ...);
    }(std::make_index_sequence<member_count_v<Ty>>());
    return tuple;
}

} // namespace internal
/*! @endcond */

template <concepts::default_reflectible_aggregate Ty>
inline const auto& offset_array_of() noexcept {
    return internal::offsets_of<Ty>();
}

template <std::size_t Index, concepts::default_reflectible_aggregate Ty>
inline auto offset_value_of() noexcept {
    return internal::offsets_of<Ty>()[Index];
}

template <concepts::default_reflectible_aggregate Ty>
inline const auto& offsets_of() noexcept {
    [[maybe_unused]] static const auto tuple = internal::offset_tuple<Ty>();
    return tuple;
}

template <std::size_t Index, concepts::default_reflectible_aggregate Ty>
inline auto offset_of() noexcept {
    static_assert(Index < member_count_v<Ty>);
    const auto& offsets = offsets_of<Ty>();
    return std::get<Index>(offsets);
}

template <tstring_v Name, concepts::default_reflectible_aggregate Ty>
inline auto offset_of() noexcept {
    constexpr auto index = index_of<Name, Ty>();
    return offset_of<index, Ty>();
}

template <concepts::has_field_traits Ty>
[[nodiscard]] consteval inline auto offsets_of() noexcept {
    constexpr auto traits  = Ty::field_traits();
    constexpr auto offsets = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
        return std::make_tuple(std::get<Is>(traits).pointer()...);
    }(std::make_index_sequence<member_count_v<Ty>>());
    return offsets;
}

template <std::size_t Index, concepts::has_field_traits Ty>
[[nodiscard]] consteval inline auto offset_of() noexcept {
    static_assert(Index < member_count_v<Ty>);
    return std::get<Index>(offsets_of<Ty>());
}

template <tstring_v Name, concepts::has_field_traits Ty>
requires(existance_of<Name, Ty>())
[[nodiscard]] constexpr inline auto offset_of() noexcept {
    return offset_of<index_of<Name, Ty>()>();
}

} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// module: member_names_of <typename>
///////////////////////////////////////////////////////////////////////////////

namespace atom::utils {

template <std::size_t Index, concepts::reflectible Ty>
struct member_type_of;

template <std::size_t Index, concepts::reflectible Ty>
requires concepts::default_reflectible_aggregate<Ty>
struct member_type_of<Index, Ty> {
    using type = std::remove_pointer_t<
        std::tuple_element_t<Index, decltype(internal::struct_to_tuple_view<Ty>())>>;
};

template <std::size_t Index, concepts::reflectible Ty>
requires concepts::has_field_traits<Ty>
struct member_type_of<Index, Ty> {
    using type = typename std::tuple_element_t<Index, decltype(Ty::field_traits())>::type;
};

template <std::size_t Index, concepts::reflectible Ty>
using member_type_of_t = typename member_type_of<Index, Ty>::type;

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

    constexpr void destroy(void* ptr) const { destroy_(ptr); }

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

private:
    static const auto& traits()
    requires concepts::default_reflectible_aggregate<Ty>
    {
        constexpr auto names    = member_names_of<Ty>();
        const auto& offsets     = offsets_of<Ty>();
        static const auto tuple = [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return std::make_tuple(
                field_traits<std::tuple_element_t<Is, std::remove_cvref_t<decltype(offsets)>>>{
                    names[Is], std::get<Is>(offsets) }...);
        }(std::make_index_sequence<member_count_v<Ty>>());
        return tuple;
    }

public:
    /**
     * @brief Fields exploded to outside in the type.
     *
     * You could get the constexpr val when Ty isn't a aggregate.
     * @return constexpr const auto& A tuple contains function traits.
     */
    constexpr const auto& fields() const noexcept {
        if constexpr (concepts::default_reflectible_aggregate<Ty>) {
            // static, but not constexpr.
            return reflected::traits();
        }
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

template <tstring_v Name, typename Tuple>
consteval std::size_t index_of(const Tuple& tuple) noexcept {
    constexpr auto index_sequence = std::make_index_sequence<std::tuple_size_v<Tuple>>();
    std::size_t result            = std::tuple_size_v<Tuple>;
    []<std::size_t... Is>(const Tuple& tuple, std::size_t& result, std::index_sequence<Is...>) {
        (internal::find_traits<Name, Is>(tuple, result), ...);
    }(tuple, result, index_sequence);
    return result;
}
} // namespace internal
/*! @endcond */

template <tstring_v Name, typename Tuple>
consteval std::size_t index_of(const Tuple& tuple) noexcept {
    constexpr auto index = index_of<Name, Tuple>(tuple);
    static_assert(index <= std::tuple_size_v<Tuple>, "there is no member named as expceted.");
    return index;
}

} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// registry & register
///////////////////////////////////////////////////////////////////////////////

#include <mutex>
#include <ranges>
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
        if (!map.contains(hash)) [[unlikely]] {
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

namespace atom::utils {
template <typename Format>
struct serialization {
    static_assert(false, "You need to create a specific version.");

    template <typename Ty>
    auto operator()(const Ty& obj, Format& ser) const -> Format& {
        return ser;
    }
    template <typename Ty>
    auto operator()(Ty& obj, Format& ser) const -> Format& {
        return ser;
    }
};

template <typename Format>
struct deserialization {
    static_assert(false, "You need to create a specific version.");

    template <typename Ty>
    auto operator()(Ty& obj, const Format& fmt) const -> Ty& {
        return obj;
    }
    template <typename Ty>
    auto operator()(Ty& obj, Format& fmt) const -> Ty& {
        return obj;
    }
};

} // namespace atom::utils

///////////////////////////////////////////////////////////////////////////////
// support for thirdparty
///////////////////////////////////////////////////////////////////////////////

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

    #define NLOHMANN_JSON_SUPPORT                                                                  \
        template <::atom::utils::concepts::reflectible Ty>                                         \
        inline void to_json(nlohmann::json& json, const Ty& obj) {                                 \
            ::to_json(json, obj);                                                                  \
        }                                                                                          \
        template <::atom::utils::concepts::reflectible Ty>                                         \
        inline void from_json(const nlohmann::json& json, Ty& obj) {                               \
            ::from_json(json, obj);                                                                \
        }                                                                                          \
        //

template <>
struct atom::utils::serialization<nlohmann::json> {
    template <typename Ty>
    auto operator()(const Ty& obj, nlohmann::json& json) const {
        if constexpr (::atom::utils::concepts::reflectible<Ty>) {
            ::to_json(json, obj);
        }
        else {
            json = obj;
        }
    }
};

template <>
struct atom::utils::deserialization<nlohmann::json> {
    template <typename Ty>
    auto operator()(Ty& obj, const nlohmann::json& json) const {
        if constexpr (::atom::utils::concepts::reflectible<Ty>) {
            ::from_json(json, obj);
        }
        else {
            obj = json;
        }
    }
};

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
auto tag_invoke(
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

template <>
struct ::atom::utils::deserialization<simdjson::fallback::ondemand::document> {
    template <typename Ty>
    auto operator()(Ty& obj, simdjson::fallback::ondemand::document& doc) const {
        ::simdjson::deserialize(doc, obj);
    }
};

#endif

#if __has_include(<lua.hpp>) && __has_include(<sol/sol.hpp>)
    #include <type_traits>
    #include <sol/sol.hpp>

namespace internal {
template <::atom::utils::concepts::reflectible Ty>
nlohmann::json to_json(const Ty& obj) {
    nlohmann::json json;
    ::to_json(json, obj);
    return json;
}

template <::atom::utils::concepts::reflectible Ty>
void from_json(Ty& obj, const nlohmann::json& json) {
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
    if constexpr (::atom::utils::concepts::reflectible<Ty>) {
        if constexpr (::atom::utils::concepts::aggregate<Ty>) {
            const auto& fields = reflected.fields();
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                ((usertype[std::get<Is>(fields).name()] = std::get<Is>(fields).pointer()), ...);
            }(std::make_index_sequence<std::tuple_size_v<decltype(fields)>>());
        }
        else if constexpr (::atom::utils::concepts::has_field_traits<Ty>) {
            constexpr auto& fields = reflected.fields();
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                ((usertype[std::get<Is>(fields).name()] = std::get<Is>(fields).pointer()), ...);
            }(std::make_index_sequence<std::tuple_size_v<decltype(fields)>>());
        }

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

///////////////////////////////////////////////////////////////////////////////
// uniform serialization & deserialization interface
///////////////////////////////////////////////////////////////////////////////

namespace atom::utils {

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
struct serialize_fn {
    template <typename Ty, typename Format>
    void operator()(const Ty& obj, Format& fmt) const {
        serialization<Format>{}(obj, fmt);
    }
};

struct deserialize_fn {
    template <typename Format>
    void operator()(auto& obj, const Format& fmt) const {
        deserialization<Format>{}(obj, fmt);
    }

    template <typename Format>
    void operator()(auto& obj, Format& fmt) const {
        deserialization<Format>{}(obj, fmt);
    }
};
} // namespace internal
/*! @endcond */

constexpr inline internal::serialize_fn serialize;
constexpr inline internal::deserialize_fn deserialize;

} // namespace atom::utils
