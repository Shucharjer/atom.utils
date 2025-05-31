#pragma once

#ifndef CPP23
    #if _HAS_CXX23 && __cplusplus >= 202302L
        #define CPP23
    #endif
#else
    #error "Macro 'CPP23' has been defined for unknown use."
#endif

#ifdef _MSC_VER
    #define ATOM_FUNCNAME __FUNCSIG__
#else
    #define ATOM_FUNCNAME __PRETTY_FUNCTION__
#endif

#ifdef _MSC_VER
    #define ATOM_FORCE_INLINE __forceinline
    #define ATOM_FORCE_NOINLINE __declspec(noinline)
    #define ALLOCATOR __declspec(allocator)
    #define NOVTABLE __declspec(novtable)
#elif defined(__GNUC__) || defined(__clang__)
    #define ATOM_FORCE_INLINE inline __attribute__((__always_inline__))
    #define ATOM_NOINLINE __attribute__((__noinline__))
    #define ALLOCATOR __attribute__((malloc))
    #define NOVTABLE
#else
    #define ATOM_FORCE_INLINE inline
    #define ATOM_NOINLINE
    #define ALLOCATOR
    #define NOVTABLE
#endif

#if defined(_DEBUG)
    #define ATOM_RELEASE_INLINE
    #define ATOM_DEBUG_SHOW_FUNC constexpr std::string_view _this_func = ATOM_FUNCNAME;
#else
    #define ATOM_RELEASE_INLINE ATOM_FORCE_INLINE
    #define ATOM_DEBUG_SHOW_FUNC
#endif

#if defined(__i386__) || defined(__x86_64__)
    #define ATOM_VECTORIZABLE true
#else
    #define ATOM_VECTORIZABLE false
#endif

#ifndef _HAS_CXX17
    #if __cplusplus > 201402L
        #define _HAS_CXX17 1
    #else
        #define _HAS_CXX17 0
    #endif
#endif

#ifndef _HAS_CXX20
    #if _HAS_CXX17 && __cplusplus > 201703L
        #define _HAS_CXX20 1
    #else
        #define _HAS_CXX20 0
    #endif
#endif

#ifndef _HAS_CXX23
    #if _HAS_CXX20 && __cplusplus > 202002L
        #define _HAS_CXX23 1
    #else
        #define _HAS_CXX23 0
    #endif
#endif

#ifndef _HAS_CXX26
    #if _HAS_CXX23 && __cplusplus > 202302L
        #define _HAS_CXX26 1
    #else
        #define _HAS_CXX26 0
    #endif
#endif

#ifndef _CONSTEXPR17
    #if _HAS_CXX17
        #define _CONSTEXPR17 constexpr
    #else
        #define _CONSTEXPR17 inline
    #endif
#endif

#ifndef _CONSTEXPR20
    #if _HAS_CXX20
        #define _CONSTEXPR20 constexpr
    #else
        #define _CONSTEXPR20 inline
    #endif
#endif

#ifndef _STATIC23
    #if _HAS_CXX23
        #define _STATIC23 static
    #else
        #define _STATIC23
    #endif
#endif

#ifndef _HAS_NODISCARD
    #ifndef __has_cpp_attribute
        #define _HAS_NODISCARD 0
    #elif __has_cpp_attribute(nodiscard) >= 201603L // TRANSITION, VSO#939899 (need toolset update)
        #define _HAS_NODISCARD 1
    #else
        #define _HAS_NODISCARD 0
    #endif
#endif

#if _HAS_NODISCARD
    #define _NODISCARD [[nodiscard]]
#else // ^^^ CAN HAZ [[nodiscard]] / NO CAN HAZ [[nodiscard]] vvv
    #define _NODISCARD
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
