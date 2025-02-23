#pragma once

#ifndef CPP23
    #if _HAS_CXX23 && __cplusplus >= 202302L
        #define CPP23
    #endif
#else
    #error "Macro 'CPP23' has been defined for unknown use."
#endif

#ifdef _MSC_VER
    #define ATOM_FORCE_INLINE __forceinline
    #define ATOM_FORCE_NOINLINE __declspec(noinline)
    #define ALLOCATOR __declspec(allocator)
#elif defined(__GNUC__) || defined(__clang__)
    #define ATOM_FORCE_INLINE inline __attribute__((__always_inline__))
    #define ATOM_NOINLINE __attribute__((__noinline__))
    #define ALLOCATOR __attribute__((malloc))
#else
    #define ATOM_FORCE_INLINE inline
    #define ATOM_NOINLINE
    #define ALLOCATOR
#endif
