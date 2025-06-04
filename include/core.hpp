/**
 * @file core.hpp
 * @author Shucharjer (Shucharjer@outlook.com)
 * @brief Core utilities library for the ATOM framework
 *
 * This header provides fundamental utilities including:
 * - Compiler-specific macro definitions for cross-platform compatibility
 * - C++ standard version feature detection macros
 * - Type-safe ID generation mechanisms
 * - Compressed pair implementations with EBCO (Empty Base Class Optimization)
 * - Polymorphic object utilities with static polymorphism
 * - Metaprogramming utilities for type and value manipulation
 *
 * Key Components:
 * 1. Compiler Feature Macros: Defines compiler-specific attributes (inlining, noinline, etc.)
 * 2. C++ Standard Detection: Macros for detecting C++17/20/23/26 features
 * 3. Type Identification: Safe runtime type ID generation
 * 4. Compressed Pairs: Memory-efficient pair implementations using EBCO
 * 5. Polymorphic Objects: Lightweight polymorphism without vtable overhead
 * 6. Metaprogramming Utilities: Type lists, value lists, and expression traits
 *
 * @date 2025-06
 * @copyright Copyright (c) 2025
 */

#ifndef _ATOM_UTILS_CORE_HPP
#define _ATOM_UTILS_CORE_HPP

// Standard library includes

#include <atomic>
#include <cstdint>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

// Compiler-specific function name macros

#ifdef _MSC_VER
    #define ATOM_FUNCNAME __FUNCSIG__         ///< MSVC function signature macro
#else
    #define ATOM_FUNCNAME __PRETTY_FUNCTION__ ///< GCC/Clang function name macro
#endif

// Compiler-specific attribute macros

#ifdef _MSC_VER
    #define ATOM_FORCE_INLINE __forceinline          ///< Force inline expansion (MSVC)
    #define ATOM_FORCE_NOINLINE __declspec(noinline) ///< Prevent inlining (MSVC)
    #define ATOM_ALLOCATOR __declspec(allocator)     ///< Function returns allocated memory (MSVC)
    #define ATOM_NOVTABLE __declspec(novtable)       ///< No vtable generation (MSVC)
#elif defined(__GNUC__) || defined(__clang__)
    #define ATOM_FORCE_INLINE                                                                      \
        inline __attribute__((__always_inline__))       ///< Force inline (GCC/Clang)
    #define ATOM_NOINLINE __attribute__((__noinline__)) ///< Prevent inlining (GCC/Clang)
    #define ATOM_ALLOCATOR __attribute__((malloc))      ///< Returns allocated memory (GCC/Clang)
    #define ATOM_NOVTABLE                               ///< Empty for GCC/Clang
#else
    #define ATOM_FORCE_INLINE inline                    ///< Fallback inline
    #define ATOM_NOINLINE                               ///< Fallback noinline
    #define ATOM_ALLOCATOR                              ///< Fallback allocator
    #define ATOM_NOVTABLE                               ///< Fallback novtable
#endif

// Debug/Release configuration

#if defined(_DEBUG)
    #define ATOM_RELEASE_INLINE                                ///< No forced inlining in debug
    #define ATOM_DEBUG_SHOW_FUNC                                                                   \
        constexpr std::string_view _this_func = ATOM_FUNCNAME; ///< Store function name
#else
    #define ATOM_RELEASE_INLINE ATOM_FORCE_INLINE              ///< Force inline in release
    #define ATOM_DEBUG_SHOW_FUNC                               ///< Empty in release
#endif

#if defined(__i386__) || defined(__x86_64__)
    #define ATOM_VECTORIZABLE true
#else
    #define ATOM_VECTORIZABLE false
#endif

// C++ standard version detection

#ifndef HAS_CXX17
    #if __cplusplus > 201402L
        #define HAS_CXX17 1 ///< C++17 supported
    #else
        #define HAS_CXX17 0 ///< C++17 not supported
    #endif
#endif

#ifndef HAS_CXX20
    #if HAS_CXX17 && __cplusplus > 201703L
        #define HAS_CXX20 1 ///< C++20 supported
    #else
        #define HAS_CXX20 0 ///< C++20 not supported
    #endif
#endif

#ifndef HAS_CXX23
    #if HAS_CXX20 && __cplusplus > 202002L
        #define HAS_CXX23 1 ///< C++23 supported
    #else
        #define HAS_CXX23 0 ///< C++23 not supported
    #endif
#endif

#ifndef HAS_CXX26
    #if HAS_CXX23 && __cplusplus > 202302L
        #define HAS_CXX26 1 ///< C++26 supported
    #else
        #define HAS_CXX26 0 ///< C++26 not supported
    #endif
#endif

// Constexpr macros per C++ standard
#ifndef CONSTEXPR17
    #if HAS_CXX17
        #define CONSTEXPR17 constexpr ///< C++17 constexpr
    #else
        #define CONSTEXPR17           ///< Empty for older standards
    #endif
#endif

#ifndef CONSTEXPR20
    #if HAS_CXX20
        #define CONSTEXPR20 constexpr ///< C++20 constexpr
    #else
        #define CONSTEXPR20           ///< Empty for older standards
    #endif
#endif

#ifndef CONSTEXPR23
    #if HAS_CXX23
        #define CONSTEXPR23 constexpr ///< C++23 constexpr
    #else
        #define CONSTEXPR23           ///< Empty for older standards
    #endif
#endif

#ifndef CONSTEXPR26
    #if HAS_CXX26
        #define CONSTEXPR26 constexpr ///< C++26 constexpr
    #else
        #define CONSTEXPR26           ///< Empty for older standards
    #endif
#endif

// Static keyword for C++23
#ifndef STATIC23
    #if HAS_CXX23
        #define STATIC23 static ///< C++23 static
    #else
        #define STATIC23        ///< Empty for older standards
    #endif
#endif

// [[nodiscard]] attribute support
#ifndef HAS_NODISCARD
    #ifndef __has_cpp_attribute
        #define _HAS_NODISCARD 0
    #elif __has_cpp_attribute(nodiscard) >= 201603L
        #define HAS_NODISCARD 1 ///< [[nodiscard]] supported
    #else
        #define HAS_NODISCARD 0 ///< [[nodiscard]] not supported
    #endif
#endif

#if HAS_NODISCARD
    #define NODISCARD [[nodiscard]] ///< Attribute to warn on unused return values
#else
    #define NODISCARD               ///< Empty when not supported
#endif

constexpr auto magic_2               = 0x2;
constexpr auto magic_4               = 0x4;
constexpr auto magic_8               = 0x8;
constexpr auto magic_16              = 0x10;
constexpr auto magic_32              = 0x20;
constexpr auto magic_64              = 0x40;
constexpr auto magic_128             = 0x80;
constexpr auto magic_256             = 0x100;
constexpr auto magic_512             = 0x200;
constexpr auto magic_1024            = 0x400;
constexpr auto magic_one_half        = 0.5F;
constexpr auto magic_double_one_half = 0.5;

namespace atom {

namespace utils {

// ID type definitions
using id_t      = uint32_t; ///< Default ID type
using long_id_t = uint64_t; ///< Extended ID type

#ifndef LONG_ID_TYPE
using default_id_t = id_t; ///< Default to 32-bit IDs
#else
using default_id_t = long_id_t; ///< Use 64-bit if requested
#endif

/**
 * @brief Template helper for passing non-type template arguments
 *
 * @tparam auto Argument value to pass
 *
 * Usage:
 *   func(spread_arg<42>);
 */
template <auto>
struct spreader {
    explicit spreader() = default;
};

template <auto Candidate>
inline constexpr spreader<Candidate> spread_arg{}; ///< Global instance

/**
 * @brief Template helper for passing type arguments
 *
 * @tparam typename Type to pass
 *
 * Usage:
 *   func(spread_type<int>);
 */
template <typename>
struct type_spreader {
    explicit type_spreader() = default;
};

template <typename Type>
inline constexpr type_spreader<Type> spread_type{}; ///< Global instance

/**
 * @brief Runtime type identification generator
 *
 * Generates unique IDs for each distinct type at runtime
 *
 * @tparam typename Placeholder for type specialization
 */
template <typename>
class type final {
public:
    type() = delete; ///< Non-instantiable

    /**
     * @brief Get unique ID for a type
     *
     * @tparam Ty Type to get ID for
     * @return default_id_t Unique type ID
     */
    template <typename Ty>
    static default_id_t id() {
        return id_<std::remove_cvref_t<Ty>>();
    }

private:
    // Internal ID generator implementation
    template <typename Ty>
    static default_id_t id_() {
        static default_id_t type_id = current_id_.fetch_add(1, std::memory_order_acquire);
        return type_id;
    }
    static inline ::std::atomic<default_id_t> current_id_; ///< Atomic ID counter
};

/**
 * @brief Non-type ID generator
 *
 * Generates unique IDs for non-type template parameters
 */
class non_type {
public:
    /**
     * @brief Get unique ID for a non-type parameter
     *
     * @tparam placeholder Unused type parameter
     * @tparam auto Non-type parameter value
     * @return default_id_t Unique ID
     */
    template <typename placeholder, auto Param>
    [[nodiscard]] static default_id_t id() {
        static default_id_t type_id = type_id_++;
        return type_id;
    }

private:
    static inline ::std::atomic<default_id_t> type_id_; ///< Atomic ID counter
};

// Type transformation utilities ---------------------------------------------

/**
 * @brief Copy const qualifier from one type to another
 *
 * @tparam To Destination type
 * @tparam From Source type for const qualification
 */
template <typename To, typename From>
struct same_constness {
    using type = To;
};
template <typename To, typename From>
struct same_constness<To, From const> {
    using type = To const;
};
template <typename To, typename From>
using same_constness_t = typename same_constness<To, From>::type;

/**
 * @brief Copy volatile qualifier from one type to another
 */
template <typename To, typename From>
struct same_volatile {
    using type = To;
};

template <typename To, typename From>
struct same_volatile<To, volatile From> {
    using type = volatile To;
};

template <typename To, typename From>
using same_volatile_t = typename same_volatile<To, From>::type;

/**
 * @brief Copy const-volatile qualifiers from one type to another
 */
template <typename To, typename From>
struct same_cv {
    using type = To;
};

template <typename To, typename From>
struct same_cv<To, const From> {
    using type = const To;
};

template <typename To, typename From>
struct same_cv<To, volatile From> {
    using type = volatile To;
};

template <typename To, typename From>
struct same_cv<To, const volatile From> {
    using type = const volatile To;
};

template <typename To, typename From>
using same_cv_t = typename same_cv<To, From>::type;

/**
 * @brief Copy reference qualifiers from one type to another
 */
template <typename To, typename From>
struct same_reference {
    using type = To;
};

template <typename To, typename From>
struct same_reference<To, From&> {
    using type = To&;
};

template <typename To, typename From>
struct same_reference<To, From&&> {
    using type = To&&;
};

template <typename To, typename From>
using same_reference_t = typename same_reference<To, From>::type;

/**
 * @brief Copy both cv and reference qualifiers
 */
template <typename To, typename From>
struct same_cvref {
    using type = To;
};

template <typename To, typename From>
struct same_cvref<To, From&> {
    using type = To&;
};

template <typename To, typename From>
struct same_cvref<To, From&&> {
    using type = To&&;
};

template <typename To, typename From>
struct same_cvref<To, const From> {
    using type = const To;
};

template <typename To, typename From>
struct same_cvref<To, const From&> {
    using type = const To&;
};

template <typename To, typename From>
struct same_cvref<To, const From&&> {
    using type = const To&&;
};

template <typename To, typename From>
struct same_cvref<To, volatile From> {
    using type = const To;
};

template <typename To, typename From>
struct same_cvref<To, volatile From&> {
    using type = const To&;
};

template <typename To, typename From>
struct same_cvref<To, volatile From&&> {
    using type = const To&&;
};

template <typename To, typename From>
struct same_cvref<To, const volatile From> {
    using type = const To;
};

template <typename To, typename From>
struct same_cvref<To, const volatile From&> {
    using type = const To&;
};

template <typename To, typename From>
struct same_cvref<To, const volatile From&&> {
    using type = const To&&;
};

template <typename To, typename From>
using same_cvref_t = typename same_cvref<To, From>::type;

template <typename Ty, typename... Tys>
concept _not = !(std::same_as<Ty, Tys> || ...);

/**
 * @brief Concept for public member pair types
 *
 * Requires first/second members and first_type/second_type typedefs
 */
template <typename Ty>
concept _public_pair = requires(const std::remove_cvref_t<Ty>& val) {
    typename std::remove_cvref_t<Ty>::first_type;
    typename std::remove_cvref_t<Ty>::second_type;
    val.first;
    val.second;
};

/**
 * @brief Concept for private member pair types
 *
 * Requires first()/second() methods and first_type/second_type typedefs
 */
template <typename Ty>
concept _private_pair = requires(const std::remove_cvref_t<Ty>& val) {
    typename std::remove_cvref_t<Ty>::first_type;
    typename std::remove_cvref_t<Ty>::second_type;
    val.first();
    val.second();
};

/**
 * @brief General pair concept
 *
 * Matches either public or private pair types
 */
template <typename Ty>
concept _pair = _public_pair<Ty> || _private_pair<Ty>;

/*! @cond TURN_OFF_DOXYGEN */

/**
 * @internal
 * @brief Storage element for compressed_pair
 *
 * Implements EBCO (Empty Base Class Optimization) for pair elements
 *
 * @tparam Ty Element type
 * @tparam IsFirst Flag indicating if this is the first element
 */
template <typename Ty, bool IsFirst>
class _compressed_element {
public:
    using self_type       = _compressed_element;
    using value_type      = Ty;
    using reference       = Ty&;
    using const_reference = const Ty&;

    /**
     * @brief Default constructor.
     *
     */
    constexpr _compressed_element() noexcept(std::is_nothrow_default_constructible_v<Ty>)
    requires std::is_default_constructible_v<Ty>
    = default;

    template <typename... Args>
    requires std::is_constructible_v<Ty, Args...>
    explicit constexpr _compressed_element(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<Ty, Args...>)
        : value_(std::forward<Args>(args)...) {}

    // clang-format off
    constexpr _compressed_element(const _compressed_element&) 
        noexcept(std::is_nothrow_copy_constructible_v<Ty>)      = default;
    constexpr _compressed_element(_compressed_element&& that) noexcept(
        std::is_nothrow_move_constructible_v<Ty>) : value_(std::move(that.value_)) {}
    constexpr _compressed_element& operator=(const _compressed_element&)
        noexcept(std::is_nothrow_copy_assignable_v<Ty>)                    = default;
    constexpr _compressed_element& operator=(_compressed_element&& that) noexcept(
        std::is_nothrow_move_constructible_v<Ty>) {
        if (this != &that) {
            value_ = std::move(that.value_);
        }
        return *this;
    }
    // clang-format on

    constexpr ~_compressed_element() noexcept(std::is_nothrow_destructible_v<Ty>) = default;

    /**
     * @brief Get the value of this element.
     *
     */
    [[nodiscard]] constexpr reference value() noexcept { return value_; }

    /**
     * @brief Get the value of this element.
     *
     */
    [[nodiscard]] constexpr const_reference value() const noexcept { return value_; }

private:
    Ty value_;
};

/**
 * @brief Compressed element. Specific version for pointer type.
 *
 * @tparam Ty Element type.
 */
template <typename Ty, bool IsFirst>
class _compressed_element<Ty*, IsFirst> {
public:
    using self_type       = _compressed_element;
    using value_type      = Ty*;
    using reference       = Ty*&;
    using const_reference = const Ty*&;

    constexpr _compressed_element() noexcept : value_(nullptr) {}
    explicit constexpr _compressed_element(std::nullptr_t) noexcept : value_(nullptr) {}
    template <typename T>
    explicit constexpr _compressed_element(T* value) noexcept : value_(value) {}
    constexpr _compressed_element(const _compressed_element&) noexcept            = default;
    constexpr _compressed_element& operator=(const _compressed_element&) noexcept = default;
    constexpr ~_compressed_element() noexcept                                     = default;

    constexpr _compressed_element& operator=(std::nullptr_t) noexcept {
        value_ = nullptr;
        return *this;
    }

    template <typename T>
    constexpr _compressed_element& operator=(T* value) noexcept {
        value_ = value;
        return *this;
    }

    constexpr _compressed_element(_compressed_element&& that) noexcept
        : value_(std::exchange(that.value_, nullptr)) {}

    constexpr _compressed_element& operator=(_compressed_element&& that) noexcept {
        value_ = std::exchange(that.value_, nullptr);
        return *this;
    }

    /**
     * @brief Get the value of this element.
     *
     */
    [[nodiscard]] constexpr reference value() noexcept { return value_; }

    /**
     * @brief Get the value of this element.
     *
     */
    [[nodiscard]] constexpr auto& value() const noexcept { return value_; }

private:
    Ty* value_;
};

/**
 * @brief When the type is void
 *
 * @tparam IsFirst
 */
template <bool IsFirst>
class _compressed_element<void, IsFirst> {
public:
    using self_type  = _compressed_element;
    using value_type = void;

    constexpr _compressed_element() noexcept                                 = default;
    constexpr _compressed_element(const _compressed_element&)                = default;
    constexpr _compressed_element(_compressed_element&&) noexcept            = default;
    constexpr _compressed_element& operator=(const _compressed_element&)     = default;
    constexpr _compressed_element& operator=(_compressed_element&&) noexcept = default;
    constexpr ~_compressed_element() noexcept                                = default;

    /**
     * @brief You may made something wrong.
     *
     */
    constexpr void value() const noexcept {}
};

template <typename Ret, typename... Args, bool IsFirst>
class _compressed_element<Ret (*)(Args...), IsFirst> {
public:
    using self_type       = _compressed_element;
    using value_type      = Ret (*)(Args...);
    using reference       = value_type&;
    using const_reference = const value_type&;

    constexpr explicit _compressed_element(const value_type value = nullptr) noexcept
        : value_(value) {}
    constexpr explicit _compressed_element(std::nullptr_t) noexcept : value_(nullptr) {}
    constexpr _compressed_element(const _compressed_element&) noexcept = default;
    constexpr _compressed_element(_compressed_element&&) noexcept      = default;
    constexpr _compressed_element& operator=(std::nullptr_t) noexcept {
        value_ = nullptr;
        return *this;
    }
    constexpr _compressed_element& operator=(const _compressed_element&) noexcept = default;
    constexpr _compressed_element& operator=(_compressed_element&& that) noexcept = default;
    constexpr ~_compressed_element() noexcept                                     = default;

    /**
     * @brief Get the value of this element.
     *
     */
    constexpr reference value() noexcept { return value_; }

    /**
     * @brief Get the value of this element.
     *
     */
    constexpr const_reference value() const noexcept { return value_; }

private:
    value_type value_;
};
/*! @endcond */

/**
 * @brief Compressed pair using EBCO (Empty Base Class Optimization)
 *
 * Memory-efficient pair implementation that uses inheritance for storage.
 * Supports structured bindings and standard pair operations.
 *
 * @tparam First Type of first element
 * @tparam Second Type of second element
 */
template <typename First, typename Second>
class compressed_pair final : private _compressed_element<First, true>,
                              private _compressed_element<Second, false> {
    using self_type   = compressed_pair;
    using first_base  = _compressed_element<First, true>;
    using second_base = _compressed_element<Second, false>;

public:
    using first_type  = First;
    using second_type = Second;

    /**
     * @brief Default constructor.
     *
     */
    constexpr compressed_pair() noexcept(
        std::is_nothrow_default_constructible_v<first_base> &&
        std::is_nothrow_default_constructible_v<second_base>)
        : first_base(), second_base() {}

    /**
     * @brief Construct each.
     *
     * @tparam FirstType
     * @tparam SecondType
     */
    template <typename FirstType, typename SecondType>
    constexpr compressed_pair(FirstType&& first, SecondType&& second) noexcept(
        std::is_nothrow_constructible_v<first_base, FirstType> &&
        std::is_nothrow_constructible_v<second_base, SecondType>)
        : first_base(std::forward<FirstType>(first)),
          second_base(std::forward<SecondType>(second)) {}

    template <typename Tuple1, typename Tuple2, size_t... Is1, size_t... Is2>
    constexpr compressed_pair(Tuple1& t1, Tuple2& t2, std::index_sequence<Is1...>, std::index_sequence<Is2...>) noexcept(
        std::is_nothrow_constructible_v<First, std::tuple_element_t<Is1, Tuple1>...> &&
        std::is_nothrow_constructible_v<Second, std::tuple_element_t<Is2, Tuple2>...>)
        : first_base(std::get<Is1>(std::move(t1))...),
          second_base(std::get<Is2>(std::move(t2))...) {}

    template <typename... Tys1, typename... Tys2>
    constexpr compressed_pair(std::piecewise_construct_t, std::tuple<Tys1...> t1, std::tuple<Tys2...> t2) noexcept(
        noexcept(compressed_pair(
            t1, t2, std::index_sequence_for<Tys1...>{}, std::index_sequence_for<Tys2...>{})))
        : compressed_pair(
              t1, t2, std::index_sequence_for<Tys1...>{}, std::index_sequence_for<Tys2...>{}) {}

    // template <
    //     typename That, typename = std::enable_if_t<!std::is_same_v<
    //                        compressed_pair, std::remove_cv_t<std::remove_reference_t<That>>>>>
    // constexpr compressed_pair(That&& that) noexcept(
    //     std::is_nothrow_constructible_v<First, same_reference_t<First, That>> &&
    //     std::is_nothrow_constructible_v<Second, same_reference_t<Second, That>>)
    //     : first_base([&] {
    //           if constexpr (concepts::public_pair<That>) {
    //               return std::forward<That>(that).first;
    //           }
    //           else if constexpr (concepts::private_pair<That>) {
    //               return std::forward<That>(that).first();
    //           }
    //           else {
    //               static_assert(false, "no suitable way to construct the first element");
    //           }
    //       }),
    //       second_base([&] {
    //           if constexpr (concepts::public_pair<That>) {
    //               return std::forward<That>(that).second;
    //           }
    //           else if constexpr (concepts::private_pair<That>) {
    //               return std::forward<That>(that).second();
    //           }
    //           else {
    //               static_assert(false, "no suitable way to construct the second element");
    //           }
    //       }) {}

    constexpr compressed_pair(const compressed_pair&)            = default;
    constexpr compressed_pair& operator=(const compressed_pair&) = default;

    constexpr compressed_pair(compressed_pair&& that) noexcept(
        std::is_nothrow_move_constructible_v<first_base> &&
        std::is_nothrow_move_constructible_v<second_base>)
        // : first_base(std::move(static_cast<first_base&&>(that))),
        //   second_base(std::move(static_cast<second_base&&>(that))) {}
        : first_base(static_cast<first_base&&>(std::move(that))),
          second_base(static_cast<second_base&&>(std::move(that))) {}

    constexpr compressed_pair& operator=(compressed_pair&& that) noexcept(
        std::is_nothrow_move_assignable_v<first_base> &&
        std::is_nothrow_move_assignable_v<second_base>) {
        static_cast<first_base&>(*this)  = static_cast<first_base&&>(std::move(that));
        static_cast<second_base&>(*this) = static_cast<second_base&&>(std::move(that));
        return *this;
    }

    constexpr ~compressed_pair() noexcept(
        std::is_nothrow_destructible_v<first_base> &&
        std::is_nothrow_destructible_v<second_base>) = default;

    [[nodiscard]] constexpr First& first() noexcept {
        return static_cast<first_base&>(*this).value();
    }

    [[nodiscard]] constexpr const First& first() const noexcept {
        return static_cast<const first_base&>(*this).value();
    }

    [[nodiscard]] constexpr Second& second() noexcept {
        return static_cast<second_base&>(*this).value();
    }

    [[nodiscard]] constexpr const Second& second() const noexcept {
        return static_cast<const second_base&>(*this).value();
    }

    constexpr operator std::pair<First, Second>() noexcept(
        std::is_nothrow_copy_constructible_v<First> && std::is_nothrow_copy_constructible_v<Second>)
    requires std::is_copy_constructible_v<First> && std::is_copy_constructible_v<Second>
    {
        return std::pair<First, Second>(first(), second());
    }
};

// if it returns true, two `compressed_pair`s must have the same tparams.
template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
[[nodiscard]] constexpr inline bool operator==(
    const compressed_pair<LFirst, LSecond>& lhs,
    const compressed_pair<RFirst, RSecond>& rhs) noexcept {
    if constexpr (std::is_same_v<LFirst, RFirst> && std::is_same_v<LSecond, RSecond>) {
        return lhs.first() == rhs.first() && lhs.second() && rhs.second();
    }
    else {
        return false;
    }
}

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
[[nodiscard]] constexpr inline bool operator!=(
    const compressed_pair<LFirst, LSecond>& lhs,
    const compressed_pair<RFirst, RSecond>& rhs) noexcept {
    return !(lhs == rhs);
}

template <typename Ty, typename First, typename Second>
[[nodiscard]] constexpr inline bool operator==(
    const compressed_pair<First, Second>& pair, const Ty& val) noexcept {
    if constexpr (std::is_convertible_v<Ty, First>) {
        return pair.first() == val;
    }
    else if constexpr (std::is_convertible_v<Ty, Second>) {
        return pair.second() == val;
    }
    else {
        return false;
    }
}

template <typename Ty, typename First, typename Second>
[[nodiscard]] constexpr inline bool operator!=(
    const compressed_pair<First, Second>& pair, const Ty& val) noexcept {
    return !(pair == val);
}

/**
 * @brief Reversed compressed pair with element order swapped
 *
 * Storage layout matches compressed_pair but logical order is reversed
 */
template <typename First, typename Second>
class reversed_compressed_pair final : private _compressed_element<Second, true>,
                                       private _compressed_element<First, false> {
    using self_type   = reversed_compressed_pair;
    using first_base  = _compressed_element<Second, true>;
    using second_base = _compressed_element<First, false>;

public:
    using first_type  = First;
    using second_type = Second;

    constexpr reversed_compressed_pair() noexcept(
        std::is_nothrow_default_constructible_v<second_base> &&
        std::is_nothrow_default_constructible_v<first_base>)
        : first_base(), second_base() {}

    template <typename FirstType, typename SecondType>
    constexpr reversed_compressed_pair(FirstType&& first, SecondType&& second) noexcept(
        std::is_nothrow_constructible_v<second_base, SecondType> &&
        std::is_nothrow_constructible_v<first_base, FirstType>)
        : first_base(std::forward<SecondType>(second)),
          second_base(std::forward<FirstType>(first)) {}

    template <typename Tuple1, typename Tuple2, size_t... Is1, size_t... Is2>
    constexpr reversed_compressed_pair(Tuple1& t1, Tuple2& t2, std::index_sequence<Is1...>, std::index_sequence<Is2...>) noexcept(
        std::is_nothrow_constructible_v<First, std::tuple_element_t<Is1, Tuple1>...> &&
        std::is_nothrow_constructible_v<Second, std::tuple_element_t<Is2, Tuple2>...>)
        : first_base(std::get<Is1>(std::move(t1))...),
          second_base(std::get<Is2>(std::move(t2))...) {}

    template <typename... Tys1, typename... Tys2>
    constexpr reversed_compressed_pair(std::piecewise_construct_t, std::tuple<Tys1...> t1, std::tuple<Tys2...> t2) noexcept(
        noexcept(reversed_compressed_pair(
            t1, t2, std::index_sequence_for<Tys1...>{}, std::index_sequence_for<Tys2...>{})))
        : reversed_compressed_pair(
              t1, t2, std::index_sequence_for<Tys1...>{}, std::index_sequence_for<Tys2...>{}) {}

    constexpr reversed_compressed_pair(const reversed_compressed_pair&)            = default;
    constexpr reversed_compressed_pair& operator=(const reversed_compressed_pair&) = default;

    constexpr reversed_compressed_pair(reversed_compressed_pair&& that) noexcept(
        std::is_nothrow_move_constructible_v<first_base> &&
        std::is_nothrow_move_constructible_v<second_base>)
        : first_base(std::move(static_cast<first_base&>(that))),
          second_base(std::move(static_cast<second_base&>(that))) {}

    constexpr reversed_compressed_pair& operator=(reversed_compressed_pair&& that) noexcept(
        std::is_nothrow_move_assignable_v<first_base> &&
        std::is_nothrow_move_assignable_v<second_base>) {
        first()  = std::move(that.first());
        second() = std::move(that.second());
        return *this;
    }

    constexpr ~reversed_compressed_pair() noexcept(
        std::is_nothrow_destructible_v<first_base> &&
        std::is_nothrow_destructible_v<second_base>) = default;

    [[nodiscard]] constexpr First& first() noexcept {
        return static_cast<second_base&>(*this).value();
    }

    [[nodiscard]] constexpr const First& first() const noexcept {
        return static_cast<const second_base&>(*this).value();
    }

    [[nodiscard]] constexpr Second& second() noexcept {
        return static_cast<first_base&>(*this).value();
    }

    [[nodiscard]] constexpr const Second& second() const noexcept {
        return static_cast<const first_base&>(*this).value();
    }
};

// if it returns true, two `reversed_compressed_pair`s must have the same tparams.
template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr inline bool operator==(
    const reversed_compressed_pair<LFirst, LSecond>& lhs,
    const reversed_compressed_pair<RFirst, RSecond>& rhs) {
    if constexpr (std::is_same_v<LFirst, RFirst> && std::is_same_v<LFirst, RFirst>) {
        return lhs.first() == rhs.first();
    }
    else {
        return false;
    }
}

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr inline bool operator!=(
    const reversed_compressed_pair<LFirst, LSecond>& lhs,
    const reversed_compressed_pair<RFirst, RSecond>& rhs) {
    return !(lhs == rhs);
}

template <typename Ty, typename First, typename Second>
constexpr inline bool operator==(
    const Ty& lhs, const reversed_compressed_pair<First, Second>& rhs) {
    if constexpr (std::is_convertible_v<Ty, First>) {
        return lhs == rhs.first();
    }
    else if constexpr (std::is_convertible_v<Ty, Second>) {
        return lhs == rhs.second();
    }
    else {
        return false;
    }
}

template <typename Ty, typename First, typename Second>
constexpr inline bool operator!=(
    const Ty& lhs, const reversed_compressed_pair<First, Second>& rhs) {
    return !(lhs == rhs);
}

template <typename First, typename Second>
struct reversed_pair {
    using first_type  = Second;
    using second_type = First;

    first_type second;
    second_type first;
};

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr inline bool operator==(
    const reversed_pair<LFirst, LSecond>& lhs, const reversed_pair<RFirst, RSecond>& rhs) {
    if constexpr (std::is_same_v<LFirst, RFirst> && std::is_same_v<LSecond, RSecond>) {
        return lhs.second == rhs.second && lhs.first == rhs.first;
    }
    else {
        return false;
    }
}

template <typename LFirst, typename LSecond, typename RFirst, typename RSecond>
constexpr inline bool operator!=(
    const reversed_pair<LFirst, LSecond>& lhs, const reversed_pair<RFirst, RSecond>& rhs) {
    return !(lhs == rhs);
}

template <typename Ty, typename First, typename Second>
constexpr inline bool operator==(const Ty& lhs, const reversed_pair<First, Second>& rhs) {
    if constexpr (std::is_convertible_v<Ty, First>) {
        return rhs.first == lhs;
    }
    else if constexpr (std::is_convertible_v<Ty, Second>) {
        return rhs.second == lhs;
    }
    else {
        return false;
    }
}

template <typename Ty, typename First, typename Second>
constexpr inline bool operator!=(const Ty& lhs, const reversed_pair<First, Second>& rhs) {
    return !(lhs == rhs);
}

template <
    typename First, typename Second, template <typename, typename> typename Pair = compressed_pair>
struct pair {
public:
    using value_type  = Pair<First, Second>;
    using first_type  = typename value_type::first_type;
    using second_type = typename value_type::second_type;

    template <typename... Args>
    requires std::is_constructible_v<value_type, Args...>
    pair(Args&&... args) noexcept(std::is_nothrow_constructible_v<value_type, Args...>)
        : pair_(std::forward<Args>(args)...) {}

    pair(const pair& that) noexcept(std::is_nothrow_copy_constructible_v<value_type>)
        : pair_(that.pair_) {}

    pair(pair&& that) noexcept(std::is_nothrow_move_constructible_v<value_type>)
        : pair_(std::move(that.pair_)) {}

    template <typename... Tys1, typename... Tys2>
    pair(std::piecewise_construct_t, std::tuple<Tys1...> t1, std::tuple<Tys2...> t2) noexcept(
        std::is_nothrow_constructible_v<
            value_type, std::piecewise_construct_t, std::tuple<Tys1...>, std::tuple<Tys2...>>)
        : pair(std::piecewise_construct, std::move(t1), std::move(t2)) {}

    pair& operator=(const pair& that) noexcept(std::is_nothrow_copy_assignable_v<value_type>) {
        if (this != &that) [[likely]] {
            pair_ = that.pair_;
        }
        return *this;
    }

    pair& operator=(pair&& that) noexcept(std::is_nothrow_move_assignable_v<value_type>) {
        if (this != &that) [[likely]] {
            pair_ = that.pair_;
        }
        return *this;
    }

    ~pair() noexcept(std::is_nothrow_destructible_v<value_type>) = default;

    constexpr First& first() noexcept {
        if constexpr (requires { pair_.first; }) {
            return pair_.first;
        }
        else if constexpr (requires { pair_.first(); }) {
            return pair_.first();
        }
        else {
            static_assert(false, "No valid way to get the first value.");
        }
    }

    constexpr const First& first() const noexcept {
        if constexpr (requires { pair_.first; }) {
            return pair_.first;
        }
        else if constexpr (requires { pair_.first(); }) {
            return pair_.first();
        }
        else {
            static_assert(false, "No valid way to get the first value.");
        }
    }

    constexpr Second& second() noexcept {
        if constexpr (requires { pair_.second; }) {
            return pair_.second;
        }
        else if constexpr (requires { pair_.second(); }) {
            return pair_.second();
        }
        else {
            static_assert(false, "No valid way to get the second value.");
        }
    }

    constexpr const Second& second() const noexcept {
        if constexpr (requires { pair_.second; }) {
            return pair_.second;
        }
        else if constexpr (requires { pair_.second(); }) {
            return pair_.second();
        }
        else {
            static_assert(false, "No valid way to get the second value.");
        }
    }

    constexpr bool operator==(const pair& that) const noexcept { return pair_ == that.pair_; }

    template <typename Ty>
    constexpr bool operator==(const Ty& that) const noexcept {
        if constexpr (requires { pair_ == that; }) {
            return pair_ == that;
        }
        else {
            return false;
        }
    }

    constexpr bool operator!=(const pair& that) const noexcept { return pair_ != that.pair_; }

    template <typename Ty>
    constexpr bool operator!=(const Ty& that) const noexcept {
        if constexpr (requires { pair_ != that; }) {
            return pair_ != that;
        }
        else {
            return false;
        }
    }

private:
    Pair<First, Second> pair_;
};

template <typename>
struct reversed_result;

template <typename First, typename Second>
struct reversed_result<compressed_pair<First, Second>> {
    using type = reversed_compressed_pair<Second, First>;
};

template <typename First, typename Second>
struct reversed_result<reversed_compressed_pair<First, Second>> {
    using type = compressed_pair<Second, First>;
};

template <typename First, typename Second>
struct reversed_result<std::pair<First, Second>> {
    using type = reversed_pair<Second, First>;
};

template <typename First, typename Second>
struct reversed_result<reversed_pair<Second, First>> {
    using type = std::pair<Second, First>;
};

template <typename First, typename Second, template <typename, typename> typename Pair>
struct reversed_result<pair<First, Second, Pair>> {
    using type = pair<Second, First, Pair>;
};

template <typename Pair>
using reversed_result_t = typename reversed_result<Pair>::type;

/*! @cond TURN_OFF_DOXYGEN */
template <typename Ty>
concept _reversible_pair = _pair<Ty> && requires {
    typename reversed_result<std::remove_cvref_t<Ty>>::type;
    std::is_trivial_v<typename std::remove_cvref_t<Ty>::first_type>;
    std::is_trivial_v<typename std::remove_cvref_t<Ty>::second_type>;
};
/*! @endcond */

/**
 * @brief Get the reversed pair.
 *
 * @tparam Pair The pair type. This tparam could be deduced automatically.
 * @param pair The pair need to reverse.
 * @return Reversed pair.
 */
template <_reversible_pair Pair>
constexpr inline decltype(auto) reverse(Pair& pair) noexcept {
    using result_type =
        typename ::atom::utils::same_cv_t<reversed_result_t<std::remove_cv_t<Pair>>, Pair>;
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<result_type&>(pair);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
}

template <size_t Index, _pair Pair>
constexpr inline decltype(auto) get(Pair& pair) noexcept {
    static_assert(Index < 2, "Pair doesn't contains so many elements.");
    if constexpr (_public_pair<Pair>) {
        if constexpr (Index == 0U)
            return pair.first;
        else
            return pair.second;
    }
    else {
        if constexpr (Index == 0U)
            return pair.first();
        else
            return pair.second();
    }
}

template <size_t Index, _pair Pair>
constexpr inline decltype(auto) get(const Pair& pair) noexcept {
    static_assert(Index < 2, "Pair doesn't contains so many elements.");
    if constexpr (_public_pair<Pair>) {
        if constexpr (Index == 0U)
            return pair.first;
        else
            return pair.second;
    }
    else {
        if constexpr (Index == 0U)
            return pair.first();
        else
            return pair.second();
    }
}

template <typename...>
struct type_list {};

template <typename>
struct type_list_size;
template <typename... Tys>
struct type_list_size<type_list<Tys...>> {
    constexpr static size_t value = sizeof...(Tys);
};
template <typename Ty>
constexpr static auto tuple_list_size_v = type_list_size<Ty>::value;

template <std::size_t Index, typename TypeList>
struct type_list_element;
template <typename Ty, typename... Args>
struct type_list_element<0, type_list<Ty, Args...>> {
    using type = Ty;
};
template <std::size_t Index, typename Ty, typename... Args>
struct type_list_element<Index, type_list<Ty, Args...>> {
    using type = type_list_element<Index - 1, Args...>;
};
template <std::size_t Index, typename TypeList>
using type_list_element_t = typename type_list_element<Index, TypeList>::type;

template <typename Ty>
struct is_type_list : std::false_type {};
template <typename... Tys>
struct is_type_list<type_list<Tys...>> : std::true_type {};
template <typename Ty>
constexpr auto is_type_list_v = is_type_list<Ty>::value;

namespace internal {

template <auto expr, typename Expr = decltype(expr)>
struct _expression_traits;

template <auto expr, typename Ret, typename... Args>
struct _expression_traits<expr, Ret (*)(Args...)> {
    constexpr static std::size_t args_count = sizeof...(Args);
    using args_type                         = type_list<Args...>;
    constexpr static bool is_constexpr =
        requires { typename std::bool_constant<(expr(Args{}...), false)>; };
};

template <auto expr, typename Ret, typename Clazz, typename... Args>
struct _expression_traits<expr, Ret (Clazz::*)(Args...)> {
    constexpr static std::size_t args_count = sizeof...(Args);
    using args_type                         = type_list<Args...>;
    constexpr static bool is_constexpr =
        requires { typename std::bool_constant<(Clazz{}.*expr(Args()...), false)>; };
};

template <auto expr, typename Ret, typename Clazz, typename... Args>
struct _expression_traits<expr, Ret (Clazz::*)(Args...) const> {
    constexpr static std::size_t args_count = sizeof...(Args);
    using args_type                         = type_list<Args...>;
    constexpr static bool is_constexpr =
        requires { typename std::bool_constant<(Clazz{}.*expr(Args{}...), false)>; };
};

template <auto expr, typename Expr>
struct _expression_traits {
    static_assert(std::is_class_v<Expr>);

    template <typename>
    struct helper;

    template <typename Ret, typename Clazz, typename... Args>
    struct helper<Ret (Clazz::*)(Args...) const> {
        constexpr static Ret invoke() { return expr(Args{}...); }

        constexpr static std::size_t args_count = sizeof...(Args);
        using args_type                         = type_list<Args...>;
        constexpr static bool is_constexpr =
            requires { typename std::bool_constant<(invoke(), false)>; };
    };

    using helper_t = helper<decltype(&Expr::operator())>;

    constexpr static std::size_t args_count = helper_t::args_count;
    using args_type                         = helper_t::args_type;
    constexpr static bool is_constexpr      = helper_t::is_constexpr;
};

}

template <auto expr>
consteval bool is_constexpr() {
    return internal::_expression_traits<expr>::is_constexpr;
}

template <typename, typename>
struct has_type : std::false_type {};
template <typename Ty, typename... Types>
struct has_type<Ty, std::tuple<Types...>> {
    constexpr static bool value = (std::is_same_v<Ty, Types> || ...);
};
template <typename Ty, typename Tuple>
constexpr bool has_type_v = has_type<Ty, Tuple>::value;

template <typename Ty, typename Tuple>
struct tuple_first;
template <typename Ty, typename... Types>
struct tuple_first<Ty, std::tuple<Types...>> {
    constexpr static size_t value = []<size_t... Is>(std::index_sequence<Is...>) {
        auto index = static_cast<size_t>(-1);
        (..., (index = (std::is_same_v<Ty, Types> ? Is : index)));
        return index;
    }(std::index_sequence_for<Types...>());
    static_assert(value < sizeof...(Types), "Type not found in tuple");
};
template <typename Ty, typename Tuple>
constexpr size_t tuple_first_v = tuple_first<Ty, Tuple>::value;

struct poly_op_copy_construct {
    void (*value)(void* ptr, const void* other) = nullptr;
};

struct poly_op_move_construct {
    void (*value)(void* ptr, void* other) = nullptr;
};

struct poly_op_copy_assign {
    void (*value)(void* ptr, const void* other) = nullptr;
};

struct poly_op_move_assign {
    void (*value)(void* ptr, void* other) = nullptr;
};

constexpr size_t k_default_poly_storage_size = 16;

constexpr size_t k_default_poly_storage_align = 16;

template <auto... Vals>
struct value_list {};

template <typename Ty>
struct value_list_size;
template <auto... Vals>
struct value_list_size<value_list<Vals...>> {
    constexpr static std::size_t value = sizeof...(Vals);
};
template <typename Ty>
constexpr size_t value_list_size_v = value_list_size<Ty>::value;

template <size_t Index, typename Ty>
struct value_list_element;
template <auto Val, auto... Others>
struct value_list_element<0, value_list<Val, Others...>> {
    constexpr static auto value = Val;
};
template <size_t Index, auto Val, auto... Others>
struct value_list_element<Index, value_list<Val, Others...>> {
    constexpr static auto value = value_list_element<Index - 1, value_list<Others...>>::value;
};
template <size_t Index, typename Ty>
constexpr inline auto value_list_element_v = value_list_element<Index, Ty>::value;

/*! @cond TURN_OFF_DOXYGEN */

template <typename>
struct _is_value_list : std::false_type {};
template <auto... Vals>
struct _is_value_list<value_list<Vals...>> : std::true_type {};
template <typename Ty>
constexpr bool _is_value_list_v = _is_value_list<Ty>::value;

/*! @endcond */

/**
 * @brief Make a tuple from a value_list.
 */
template <typename Ty>
requires _is_value_list_v<Ty>
constexpr auto make_tuple() noexcept {
    return []<size_t... Is>(std::index_sequence<Is...>) {
        return std::make_tuple(value_list_element_v<Is, Ty>...);
    }(std::make_index_sequence<value_list_size_v<Ty>>());
}
/**
 * @brief Make a tuple from a value_list.
 */
template <typename Ty>
requires _is_value_list_v<Ty>
constexpr auto make_tuple(Ty) noexcept {
    return []<size_t... Is>(std::index_sequence<Is...>) {
        return std::make_tuple(value_list_element_v<Is, Ty>...);
    }(std::make_index_sequence<value_list_size_v<Ty>>());
}

/*! @cond TURN_OFF_DOXYGEN */

struct _poly_empty_impl {
    struct _universal {
#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wreturn-type"
#elif defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wreturn-type"
#elif defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable : 4715)
#endif
        template <typename Ty>
        constexpr operator Ty&&() {}
#ifdef __GNUC__
    #pragma GCC diagnostic pop
#elif defined(__clang__)
    #pragma clang diagnostic pop
#elif defined(_MSC_VER)
    #pragma warning(pop)
#endif
    };

    template <size_t Index, typename... Args>
    constexpr inline auto invoke(Args&&...) noexcept {
        return _universal{};
    }
    template <size_t Index, typename... Args>
    constexpr inline auto invoke(Args&&...) const noexcept {
        return _universal{};
    }
};

template <typename Ty>
concept _poly_basic_object = requires {
    // Ty should declared a class template called interface.
    typename Ty::template interface<_poly_empty_impl>;
    requires std::is_base_of_v<_poly_empty_impl, typename Ty::template interface<_poly_empty_impl>>;

    // Ty should declared a alias which is a value_list called interface.
    typename Ty::template impl<typename Ty::template interface<_poly_empty_impl>>;
    requires _is_value_list_v<typename Ty::template impl<typename Ty::template interface<_poly_empty_impl>>>;
};

namespace internal {
template <typename Ty>
concept _poly_has_extend = requires {
    typename Ty::template extends<_poly_empty_impl>;
    requires std::is_base_of_v<_poly_empty_impl, typename Ty::template extends<_poly_empty_impl>>;

    typename Ty::template impl<typename Ty::template extends<_poly_empty_impl>>;
};
}

namespace internal {
template <typename>
struct poly_assert;
template <typename... Tys>
struct poly_assert<type_list<Tys...>> {
    constexpr static bool value = (poly_assert<Tys>::value && ...);
};
template <typename Ty>
struct poly_assert {
    constexpr static bool value = [](){
        if constexpr (_poly_basic_object<Ty>) {
            return true;
        }
        else if constexpr (internal::_poly_has_extend<Ty>) {
            return poly_assert<Ty>::value;
        }
        else {
            return false;
        }
    };
};
}

template <typename Ty>
concept _poly_extend_object = requires {
    typename Ty::template extends<_poly_empty_impl>;
    requires std::is_base_of_v<_poly_empty_impl, typename Ty::template extends<_poly_empty_impl>>;
    typename Ty::template extends<_poly_empty_impl>::from;
    requires internal::poly_assert<typename Ty::template extends<_poly_empty_impl>::from>::value;
};

template <typename Ty>
concept _poly_object = _poly_basic_object<Ty> || _poly_extend_object<Ty>;

template <typename Ty, typename Object>
concept _poly_impl = requires {
    // The template argument `Object` should satisfy _poly_object
    requires _poly_object<Object>;

    // `Ty` should has functions required in Object impl
    typename Object::template impl<std::remove_cvref_t<Ty>>;
} && !requires {
    // The type has a class or alias named 'poly_tag' is already a polymorphic object or
    // the designer does not whish you do this.
    // You should not pass thru this type as a polymorphic implementation.
    typename Ty::poly_tag;
};

template <typename>
struct _mem_func_traits;
template <typename Ret, typename Class, typename... Args>
struct _mem_func_traits<Ret (Class::*)(Args...)> {
    using type                        = Ret (*)(Class&, Args...);
    using return_type                 = Ret;
    using class_type                  = Class;
    using args_type                   = std::tuple<Args...>;
    using static_type                 = Ret (*)(void*, Args...);
    constexpr static bool is_noexcept = false;
};
template <typename Ret, typename Class, typename... Args>
struct _mem_func_traits<Ret (Class::*)(Args...) const> {
    using type                        = Ret (*)(const Class&, Args...);
    using return_type                 = Ret;
    using class_type                  = Class;
    using args_type                   = std::tuple<Args...>;
    using static_type                 = Ret (*)(const void*, Args...);
    constexpr static bool is_noexcept = false;
};
template <typename Ret, typename Class, typename... Args>
struct _mem_func_traits<Ret (Class::*)(Args...) noexcept> {
    using type                        = Ret (*)(Class&, Args...);
    using return_type                 = Ret;
    using class_type                  = Class;
    using args_type                   = std::tuple<Args...>;
    using static_type                 = Ret (*)(void*, Args...) noexcept;
    constexpr static bool is_noexcept = true;
};
template <typename Ret, typename Class, typename... Args>
struct _mem_func_traits<Ret (Class::*)(Args...) const noexcept> {
    using type                        = Ret (*)(const Class&, Args...);
    using return_type                 = Ret;
    using class_type                  = Class;
    using args_type                   = std::tuple<Args...>;
    using static_type                 = Ret (*)(const void*, Args...) noexcept;
    constexpr static bool is_noexcept = true;
};

template <_poly_object Object>
struct _vtable;

template <_poly_object Object>
requires _poly_basic_object<Object>
struct _vtable<Object> {
    using _empty_interface     = typename Object::template interface<_poly_empty_impl>;
    using _empty_impl          = typename Object::template impl<_empty_interface>;
    constexpr static auto size = value_list_size_v<_empty_impl>;

    template <typename MemFunc>
    using static_type_t = typename _mem_func_traits<MemFunc>::static_type;

    template <auto... Vals>
    constexpr static auto _deduce(value_list<Vals...>) noexcept {
        return static_cast<std::tuple<static_type_t<decltype(Vals)>...>*>(nullptr);
    }
    constexpr static auto _deduce() noexcept { return _deduce(_empty_impl{}); }

    using type = std::remove_pointer_t<decltype(_deduce())>;
};

template <_poly_object Object>
requires _poly_extend_object<Object>
struct _vtable<Object> {
    using _empty_interface     = typename Object::template extends<_poly_empty_impl>;
    using _empty_impl          = typename Object::template impl<_empty_interface>;
    constexpr static auto size = value_list_size_v<_empty_impl>;

    template <typename MemFunc>
    using static_type_t = typename _mem_func_traits<MemFunc>::static_type;

    template <auto... Vals>
    constexpr static auto _deduce(value_list<Vals...>) noexcept {
        return static_cast<std::tuple<static_type_t<decltype(Vals)>...>*>(nullptr);
    }
    constexpr static auto _deduce() noexcept { return _deduce(_empty_impl{}); }

    using type = std::remove_pointer_t<decltype(_deduce())>;
};

/*! @cond TURN_OFF_DOXYGEN */

template <_poly_object Object>
using vtable = typename _vtable<Object>::type;

/*! @cond TURN_OFF_DOXYGEN */

template <_poly_object Object, _poly_impl<Object> Impl>
struct _vtable_value_list {
    template <typename ValueList>
    struct _static_list;

    template <auto... Vals>
    struct _static_list<value_list<Vals...>> {
        template <auto, typename>
        struct _element;
        template <auto Val, typename Ret, typename Class, typename... Args>
        struct _element<Val, Ret (Class::*)(Args...)> {
            constexpr static Ret (*value)(void*, Args...) = [](void* ptr, Args... args) {
                return (static_cast<Class*>(ptr)->*Val)(std::forward<Args>(args)...);
            };
        };
        template <auto Val, typename Ret, typename Class, typename... Args>
        struct _element<Val, Ret (Class::*)(Args...) const> {
            constexpr static Ret (*value)(const void*, Args...) = [](const void* ptr,
                                                                     Args... args) {
                return (static_cast<const Class*>(ptr)->*Val)(std::forward<Args>(args)...);
            };
        };
        template <auto Val, typename Ret, typename Class, typename... Args>
        struct _element<Val, Ret (Class::*)(Args...) noexcept> {
            constexpr static Ret (*value)(void*, Args...) noexcept = [](void* ptr,
                                                                        Args... args) noexcept {
                return (static_cast<Class*>(ptr)->*Val)(std::forward<Args>(args)...);
            };
        };
        template <auto Val, typename Ret, typename Class, typename... Args>
        struct _element<Val, Ret (Class::*)(Args...) const noexcept> {
            constexpr static Ret (*value)(const void*, Args...) noexcept =
                [](const void* ptr, Args... args) noexcept {
                    return (static_cast<const Class*>(ptr)->*Val)(std::forward<Args>(args)...);
                };
        };
        template <auto Val, typename Func>
        constexpr static auto _element_v = _element<Val, Func>::value;

        using type = value_list<_element_v<Vals, decltype(Vals)>...>;
    };

    using type = typename _static_list<typename Object::template impl<Impl>>::type;
};

template <_poly_object Object, _poly_impl<Object> Impl>
using _vtable_value_list_t = _vtable_value_list<Object, Impl>::type;

/*! @endcond */

template <_poly_object Object, _poly_impl<Object> Impl>
consteval inline auto make_vtable() noexcept {
    return make_tuple<_vtable_value_list_t<Object, Impl>>();
}

template <_poly_object Object, _poly_impl<Object> Impl>
constexpr static inline auto static_vtable = make_vtable<Object, Impl>();

template <typename Poly>
class poly_base;

// polymorphic<Object> ->  Object::temlate interface<Impl> -> polymorphic_base<polymorphic<Object>

/**
 * @brief Static polymorphic object container
 *
 * Implements runtime polymorphism without traditional vtable overhead
 *
 * @tparam Object Polymorphic object type defining the interface
 * @tparam Size Storage buffer size
 * @tparam Align Storage buffer alignment
 * @tparam Ops Custom operations tuple
 */
template <
    _poly_object Object, size_t Size = k_default_poly_storage_size,
    size_t Align = k_default_poly_storage_align, typename Ops = std::tuple<>>
class poly;

template <
    _poly_object Object, size_t Size, size_t Align, typename Ops>
requires _poly_basic_object<Object>
class poly<Object, Size, Align, Ops> : private Object::template interface<poly_base<poly<Object, Size, Align, Ops>>> {

    // Friend declaration for CRTP

    friend class poly_base<poly>;

    // Internal type aliases

    struct _poly_operation_destroy {
        void (*value)(void* ptr) = nullptr;
    };

    using _empty_interface = typename Object::template interface<_poly_empty_impl>;
    using vtable_t           = vtable<Object>;
    using ops_t            = decltype(std::tuple_cat(Ops{}, std::tuple<_poly_operation_destroy>{}));

    // Storage management

    constexpr static size_t _store_vtable_as_pointer = sizeof(vtable_t) > sizeof(void*) * 2;

    ///< Pointer to stored object
    void* ptr_;
    ///< Custom operations
    ops_t operations_;

    /// The vtable for the polymorphic object.
    std::conditional_t<_store_vtable_as_pointer, const vtable_t*, vtable_t> vtable_;

    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
    // NOLINTBEGIN(modernize-avoid-c-arrays)

    /// Internal storage buffer
    alignas(Align) std::byte storage_[Size];

    // NOLINTEND(modernize-avoid-c-arrays)
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays)

    /// @brief return compile-time virtual table value.
    template <_poly_impl<Object> Impl>
    consteval static auto _vtable_value() noexcept {
        if constexpr (_store_vtable_as_pointer) {
            return &static_vtable<Object, Impl>;
        }
        else {
            return make_vtable<Object, Impl>();
        }
    }

    template <_poly_impl<Object> Impl>
    constexpr static bool builtin_storable = sizeof(Impl) <= Size && alignof(Impl) <= Align;

    template <_poly_impl<Object> Impl>
    constexpr void* _init_ptr() {
        if constexpr (builtin_storable<Impl>) {
            return static_cast<void*>(storage_);
        }
        else {
            return operator new(sizeof(Impl), std::align_val_t(Align));
        }
    }

    template <_poly_impl<Object> Impl>
    constexpr void construct(Impl&& impl) {
        ::new (ptr_) Impl(std::forward<Impl>(impl));
        ptr_ = std::launder(static_cast<Impl*>(ptr_));
    }

    constexpr void destroy() {
        std::get<tuple_first_v<_poly_operation_destroy, ops_t>>(operations_).value(ptr_);
    }

    template <_poly_impl<Object> Impl>
    constexpr static inline ops_t _operations() noexcept {
        ops_t ops;
        if constexpr (has_type_v<poly_op_copy_construct, ops_t>) {
            std::get<tuple_first_v<poly_op_copy_construct, ops_t>>(ops).value =
                [](void* lhs,
                   const void* rhs) noexcept(std::is_nothrow_copy_constructible_v<Impl>) {
                    ::new (lhs) Impl(*static_cast<const Impl*>(rhs));
                };
        }
        if constexpr (has_type_v<poly_op_move_construct, ops_t>) {
            std::get<tuple_first_v<poly_op_move_construct, ops_t>>(ops).value =
                [](void* lhs, void* rhs) noexcept(std::is_nothrow_move_constructible_v<Impl>) {
                    ::new (lhs) Impl(std::move(*static_cast<Impl*>(rhs)));
                };
        }
        if constexpr (has_type_v<poly_op_copy_assign, ops_t>) {
            std::get<tuple_first_v<poly_op_copy_assign, ops_t>>(ops).value =
                [](void* lhs, const void* rhs) noexcept(std::is_nothrow_copy_assignable_v<Impl>) {
                    *static_cast<Impl*>(lhs) = *static_cast<const Impl*>(rhs);
                };
        }
        if constexpr (has_type_v<poly_op_move_assign, ops_t>) {
            std::get<tuple_first_v<poly_op_move_assign, ops_t>>(ops).value =
                [](void* lhs, void* rhs) noexcept(std::is_nothrow_move_assignable_v<Impl>) {
                    *static_cast<Impl*>(lhs) = std::move(*static_cast<Impl*>(rhs));
                };
        }
        std::get<tuple_first_v<_poly_operation_destroy, ops_t>>(ops).value =
            [](void* ptr) noexcept(std::is_nothrow_destructible_v<Impl>) {
                static_cast<Impl*>(ptr)->~Impl();
            };
        return ops;
    }

public:
    using interface = typename Object::template interface<poly_base<poly>>;

    /**
     * @brief Constructs a polymorphic object with an empty interface.
     * @warning This constructor initializes the polymorphic object with an empty interface. You
     * should construct the polymorphic object with a valid implementation before using it.
     */
    constexpr poly() noexcept
        : ptr_(nullptr), vtable_(_vtable_value<_empty_interface>()), storage_(), operations_() {}

    template <_poly_impl<Object> Impl>
    constexpr poly(Impl&& impl)
        : ptr_(_init_ptr<Impl>()), vtable_(_vtable_value<Impl>()), storage_(),
          operations_(_operations<Impl>()) {
        construct(std::forward<Impl>(impl));
    }

    poly(const poly& that)
    requires has_type_v<poly_op_copy_construct, ops_t>
    {
        // TODO:
    }

    poly(poly&& that)
    requires has_type_v<poly_op_move_construct, ops_t>
    {
        // TODO:
    }

    poly& operator=(const poly& that)
    requires has_type_v<poly_op_copy_assign, ops_t>
    {
        // TODO:
    }

    poly& operator=(poly&& that)
    requires has_type_v<poly_op_move_assign, ops_t>
    {
        // TODO:
    }

    constexpr ~poly() {
        if (ptr_) [[likely]] {
            destroy();
            ptr_ = nullptr;
        }
    }

    [[nodiscard]] constexpr inline interface* operator->() noexcept { return this; }
    [[nodiscard]] constexpr inline const interface* operator->() const noexcept { return this; }

    [[nodiscard]] constexpr inline operator bool() const noexcept { return ptr_; }

    [[nodiscard]] constexpr inline void* data() noexcept { return ptr_; }
    [[nodiscard]] constexpr inline const void* data() const noexcept { return ptr_; }
};

template <
    _poly_object Object, size_t Size, size_t Align, typename Ops>
requires _poly_extend_object<Object>
class poly<Object, Size, Align, Ops> : private Object::template extends<poly_base<poly<Object, Size, Align, Ops>>> {

    // Friend declaration for CRTP

    friend class poly_base<poly>;

    // Internal type aliases

    struct _poly_operation_destroy {
        void (*value)(void* ptr) = nullptr;
    };

    using _empty_interface = typename Object::template extends<_poly_empty_impl>;
    using vtable_t           = vtable<Object>;
    using ops_t            = decltype(std::tuple_cat(Ops{}, std::tuple<_poly_operation_destroy>{}));

    // Storage management

    constexpr static size_t _store_vtable_as_pointer = sizeof(vtable_t) > sizeof(void*) * 2;

    ///< Pointer to stored object
    void* ptr_;
    ///< Custom operations
    ops_t operations_;

    /// The vtable for the polymorphic object.
    std::conditional_t<_store_vtable_as_pointer, const vtable_t*, vtable_t> vtable_;

    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
    // NOLINTBEGIN(modernize-avoid-c-arrays)

    /// Internal storage buffer
    alignas(Align) std::byte storage_[Size];

    // NOLINTEND(modernize-avoid-c-arrays)
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays)

    /// @brief return compile-time virtual table value.
    template <_poly_impl<Object> Impl>
    consteval static auto _vtable_value() noexcept {
        if constexpr (_store_vtable_as_pointer) {
            return &static_vtable<Object, Impl>;
        }
        else {
            return make_vtable<Object, Impl>();
        }
    }

    template <_poly_impl<Object> Impl>
    constexpr static bool builtin_storable = sizeof(Impl) <= Size && alignof(Impl) <= Align;

    template <_poly_impl<Object> Impl>
    constexpr void* _init_ptr() {
        if constexpr (builtin_storable<Impl>) {
            return static_cast<void*>(storage_);
        }
        else {
            return operator new(sizeof(Impl), std::align_val_t(Align));
        }
    }

    template <_poly_impl<Object> Impl>
    constexpr void construct(Impl&& impl) {
        ::new (ptr_) Impl(std::forward<Impl>(impl));
        ptr_ = std::launder(static_cast<Impl*>(ptr_));
    }

    constexpr void destroy() {
        std::get<tuple_first_v<_poly_operation_destroy, ops_t>>(operations_).value(ptr_);
    }

    template <_poly_impl<Object> Impl>
    constexpr static inline ops_t _operations() noexcept {
        ops_t ops;
        if constexpr (has_type_v<poly_op_copy_construct, ops_t>) {
            std::get<tuple_first_v<poly_op_copy_construct, ops_t>>(ops).value =
                [](void* lhs,
                   const void* rhs) noexcept(std::is_nothrow_copy_constructible_v<Impl>) {
                    ::new (lhs) Impl(*static_cast<const Impl*>(rhs));
                };
        }
        if constexpr (has_type_v<poly_op_move_construct, ops_t>) {
            std::get<tuple_first_v<poly_op_move_construct, ops_t>>(ops).value =
                [](void* lhs, void* rhs) noexcept(std::is_nothrow_move_constructible_v<Impl>) {
                    ::new (lhs) Impl(std::move(*static_cast<Impl*>(rhs)));
                };
        }
        if constexpr (has_type_v<poly_op_copy_assign, ops_t>) {
            std::get<tuple_first_v<poly_op_copy_assign, ops_t>>(ops).value =
                [](void* lhs, const void* rhs) noexcept(std::is_nothrow_copy_assignable_v<Impl>) {
                    *static_cast<Impl*>(lhs) = *static_cast<const Impl*>(rhs);
                };
        }
        if constexpr (has_type_v<poly_op_move_assign, ops_t>) {
            std::get<tuple_first_v<poly_op_move_assign, ops_t>>(ops).value =
                [](void* lhs, void* rhs) noexcept(std::is_nothrow_move_assignable_v<Impl>) {
                    *static_cast<Impl*>(lhs) = std::move(*static_cast<Impl*>(rhs));
                };
        }
        std::get<tuple_first_v<_poly_operation_destroy, ops_t>>(ops).value =
            [](void* ptr) noexcept(std::is_nothrow_destructible_v<Impl>) {
                static_cast<Impl*>(ptr)->~Impl();
            };
        return ops;
    }

public:
    using interface = typename Object::template extends<poly_base<poly>>;

    /**
     * @brief Constructs a polymorphic object with an empty interface.
     * @warning This constructor initializes the polymorphic object with an empty interface. You
     * should construct the polymorphic object with a valid implementation before using it.
     */
    constexpr poly() noexcept
        : ptr_(nullptr), vtable_(_vtable_value<_empty_interface>()), storage_(), operations_() {}

    template <_poly_impl<Object> Impl>
    constexpr poly(Impl&& impl)
        : ptr_(_init_ptr<Impl>()), vtable_(_vtable_value<Impl>()), storage_(),
          operations_(_operations<Impl>()) {
        construct(std::forward<Impl>(impl));
    }

    poly(const poly& that)
    requires has_type_v<poly_op_copy_construct, ops_t>
    {
        // TODO:
    }

    poly(poly&& that)
    requires has_type_v<poly_op_move_construct, ops_t>
    {
        // TODO:
    }

    poly& operator=(const poly& that)
    requires has_type_v<poly_op_copy_assign, ops_t>
    {
        // TODO:
    }

    poly& operator=(poly&& that)
    requires has_type_v<poly_op_move_assign, ops_t>
    {
        // TODO:
    }

    constexpr ~poly() {
        if (ptr_) [[likely]] {
            destroy();
            ptr_ = nullptr;
        }
    }

    [[nodiscard]] constexpr inline interface* operator->() noexcept { return this; }
    [[nodiscard]] constexpr inline const interface* operator->() const noexcept { return this; }

    [[nodiscard]] constexpr inline operator bool() const noexcept { return ptr_; }

    [[nodiscard]] constexpr inline void* data() noexcept { return ptr_; }
    [[nodiscard]] constexpr inline const void* data() const noexcept { return ptr_; }
};

template <
    _poly_object Object, _poly_impl<Object> Impl, size_t Size = k_default_poly_storage_size,
    size_t Align = k_default_poly_storage_align, typename Ops = std::tuple<>, typename... Args>
requires std::constructible_from<Impl, Args...>
constexpr inline poly<Object, Size, Align, Ops> make_poly(Args&&... args) {
    // TODO:
}

template <typename Poly>
class poly_base;

template <_poly_object Object, size_t Size, size_t Align, typename Ops>
class poly_base<poly<Object, Size, Align, Ops>> {
public:
    using polymorphic_type = poly<Object, Size, Align, Ops>;

    template <size_t Index, typename... Args>
    constexpr inline decltype(auto) invoke(Args&&... args) {
        auto* const ptr = static_cast<polymorphic_type*>(this);
        if constexpr (polymorphic_type::_store_vtable_as_pointer) {
            return std::get<Index>(*(ptr->vtable_))(ptr->ptr_, std::forward<Args>(args)...);
        }
        else {
            return std::get<Index>(ptr->vtable_)(ptr->ptr_, std::forward<Args>(args)...);
        }
    }

    template <size_t Index, typename... Args>
    constexpr inline decltype(auto) invoke(Args&&... args) const {
        auto* const ptr = static_cast<const polymorphic_type*>(this);
        if constexpr (polymorphic_type::_store_vtable_as_pointer) {
            return std::get<Index>(*(ptr->vtable_))(ptr->ptr_, std::forward<Args>(args)...);
        }
        else {
            return std::get<Index>(ptr->vtable_)(ptr->ptr_, std::forward<Args>(args)...);
        }
    }
};

} // namespace utils

using default_id_t = utils::default_id_t;

} // namespace atom

/*! @cond TURN_OFF_DOXYGEN */

namespace std {

template <typename Pair>
requires ::atom::utils::_pair<Pair>
struct tuple_size<Pair> : std::integral_constant<size_t, 2> {};

template <size_t Index, typename Pair>
requires ::atom::utils::_pair<Pair>
struct tuple_element<Index, Pair> {
    static_assert(Index < 2);
    using type =
        std::conditional_t<(Index == 0), typename Pair::first_type, typename Pair::second_type>;
};

template <size_t Index, ::atom::utils::_pair Pair>
constexpr inline decltype(auto) get(Pair& pair) noexcept {
    static_assert(Index < 2, "Pair doesn't contains so many elements.");
    if constexpr (::atom::utils::_public_pair<Pair>) {
        if constexpr (Index == 0U)
            return pair.first;
        else
            return pair.second;
    }
    else {
        if constexpr (Index == 0U)
            return pair.first();
        else
            return pair.second();
    }
}

template <size_t Index, ::atom::utils::_pair Pair>
constexpr inline decltype(auto) get(const Pair& pair) noexcept {
    static_assert(Index < 2, "Pair doesn't contains so many elements.");
    if constexpr (::atom::utils::_public_pair<Pair>) {
        if constexpr (Index == 0U)
            return pair.first;
        else
            return pair.second;
    }
    else {
        if constexpr (Index == 0U)
            return pair.first();
        else
            return pair.second();
    }
}

} // namespace std

/*! @endcond */

#endif
