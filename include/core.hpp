#pragma once
#include <type_traits>

#define UTILS ::atom::utils::

namespace atom {

namespace utils {

using id_t      = uint32_t;
using long_id_t = uint64_t;

#ifndef LONG_ID_TYPE
using default_id_t = id_t;
#else
using default_id_t = long_id_t;
#endif

template <typename, typename>
class compressed_pair;

template <typename, typename>
class reversed_compressed_pair;

template <typename, typename>
struct reversed_pair;

template <typename Rng, typename Closure>
struct pipline_result;

template <typename Fn, typename... Args>
class closure;

template <typename Fn, typename... Args>
constexpr static auto make_closure(Args&&... args) -> closure<Fn, std::decay_t<Args>...>;

template <auto>
struct spreader;

template <typename>
struct type_spreader;

template <typename = void>
class type;

} // namespace utils

using default_id_t = utils::default_id_t;

} // namespace atom
