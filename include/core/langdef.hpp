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
    #define ATOM_BEBUG_SHOW_FUNC
#endif

#if defined(__i386__) || defined(__x86_64__)
    #define ATOM_VECTORIZABLE true
#else
    #define ATOM_VECTORIZABLE false
#endif

constexpr auto num_eight      = 8;
constexpr auto num_sixteen    = 16;
constexpr auto num_thirty_two = 32;
