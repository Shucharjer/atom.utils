#pragma once
#include <cstdint>

namespace atom::utils {

template <typename Ty>
struct half_size;

template <>
struct half_size<uint64_t> {
    using type = uint32_t;
};

template <>
struct half_size<int64_t> {
    using type = int32_t;
};

template <>
struct half_size<uint32_t> {
    using type = uint16_t;
};

template <>
struct half_size<int32_t> {
    using type = int16_t;
};

template <>
struct half_size<uint16_t> {
    using type = uint8_t;
};

template <>
struct half_size<int16_t> {
    using type = int8_t;
};

template <typename Ty>
using half_size_t = typename half_size<Ty>::type;

} // namespace atom::utils
