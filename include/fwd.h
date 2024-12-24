#pragma once
#include <cstddef>
#include <cstdint>

namespace atom {

namespace utils {
using id_t      = uint32_t;
using id_long_t = uint64_t;

#ifdef LONG_ID_TYPE
using default_id_t = id_long_t;
#else
using default_id_t = id_t;
#endif

template <auto>
struct spreader;

template <typename>
struct type_spreader;

class type;

} // namespace utils

using default_id_t = utils::default_id_t;

} // namespace atom
