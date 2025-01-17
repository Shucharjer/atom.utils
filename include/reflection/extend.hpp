#pragma once
#include "core.hpp"
#include "reflection.hpp"

namespace atom::utils {

struct extend {
    // clang-format off
    void    (*construct)                (void* ptr)                  = nullptr;
    void    (*destroy)                  (void* ptr)                  = nullptr;
    void*   (*new_object)               ()                           = nullptr;
    void    (*delete_object)            (void* ptr)                  = nullptr;
    void*   (*new_object_in_pool)       (void* pool)                 = nullptr;
    void    (*delete_object_in_pool)    (void* ptr, void* pool)      = nullptr;
    void    (*serialize)                (void* dst, const void* src) = nullptr;
    void    (*deserialize)              (const void* src, void* dst) = nullptr;
    // clang-format on
};

} // namespace atom::utils
