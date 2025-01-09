#pragma once

#ifndef UTILS
    #define UTILS ::atom::utils::
#else
    #error "Macro 'AUTILS' has been defined for unknown reason."
#endif

#ifndef CPP23
    #if _HAS_CXX23 && __cplusplus >= 202302L
        #define CPP23
    #endif
#else
    #error "Macro 'CPP23' has been defined for unknown reason."
#endif
