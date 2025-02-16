#pragma once
#include "reflection/aggregate/member_count.hpp"
#include "reflection/others/member_count.hpp"

namespace atom::utils {
template <typename Ty>
requires std::is_aggregate_v<Ty> || concepts::has_field_traits<Ty>
constexpr inline size_t member_count_v = member_count_of<Ty>();
}
