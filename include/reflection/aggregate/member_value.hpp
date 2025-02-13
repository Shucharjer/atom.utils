#pragma once
#include <algorithm>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include "concepts/type.hpp"
#include "member_count.hpp"
#include "reflection/aggregate/tuple_view.hpp"

namespace atom::utils {

template <size_t Index, concepts::aggregate Ty>
constexpr inline auto& get(Ty& obj) {
    static_assert(Index < member_count_v<Ty>);
    auto tuple = internal::object_to_tuple_view(obj);
    return std::get<Index>(tuple);
}

template <concepts::aggregate Ty>
inline auto offsets_of() noexcept -> const std::array<size_t, member_count_v<Ty>>& {
    auto& outline        = internal::get_object_outline<Ty>();
    auto tuple           = internal::object_to_tuple_view(outline);
    constexpr auto count = member_count_v<Ty>;

    // why we couldn't use reinterpret_cast in constexpr?
    [[maybe_unused]] static std::array<size_t, count> array = {
        [&]<size_t... Is>(std::index_sequence<Is...>) {
            std::array<size_t, count> arr;
            ((arr[Is] = size_t((const char*)&std::get<Is>(tuple) - (char*)&outline)), ...);
            return arr;
        }(std::make_index_sequence<count>())
    };
    return array;
}

} // namespace atom::utils

#if __has_include(<nlohmann/json.hpp>)
    #include <nlohmann/json.hpp>

/**
 * @brief Serialization support for nlohmann-json.
 *
 */
template <atom::utils::concepts::aggregate Ty>
constexpr inline auto to_json(nlohmann::json& json, const Ty& obj) {
    auto names = atom::utils::member_names_of<Ty>();
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((json[names[Is]] = atom::utils::get<Is>(obj)), ...);
    }(std::make_index_sequence<atom::utils::member_count_v<Ty>>());
}

/**
 * @brief Deserialization support for nlohmann-json.
 *
 */
template <atom::utils::concepts::aggregate Ty>
constexpr inline auto from_json(const nlohmann::json& json, Ty& obj) {
    auto names = atom::utils::member_names_of<Ty>();
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((json.at(names[Is]).get_to(atom::utils::get<Is>(obj))), ...);
    }(std::make_index_sequence<::atom::utils::member_count_v<Ty>>());
}

#endif

#ifndef __GNUC__
    #if __has_include(<simdjson.h>)
        #include <string>
        #include <string_view>
        #include <simdjson.h>

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
template <size_t Index, typename Ty, typename Val>
inline auto tag_invoke_impl(Ty&& obj, Val& val) {
    using type = std::remove_cvref_t<Val>;
    if constexpr (std::is_same_v<type, uint64_t>) {
        return obj.get_uint64(val);
    }
    else if constexpr (std::is_same_v<type, int64_t>) {
        return obj.get_int64(val);
    }
    else if constexpr (std::is_same_v<type, double>) {
        return obj.get_double(val);
    }
    else if constexpr (std::is_same_v<type, bool>) {
        return obj.get_bool(val);
    }
    else if constexpr (std::is_same_v<type, std::string_view> ||
                       std::is_same_v<type, std::string>) {
        return obj.get_string(val);
    }
    else if constexpr (std::is_same_v<type, simdjson::ondemand::object>) {
        return obj.get_object(val);
    }
    else if constexpr (std::is_same_v<type, simdjson::ondemand::array>) {
        return obj.get_array(val);
    }
    else {
        return obj.get(val);
    }
}
} // namespace internal
/*! @endcond */

namespace simdjson {
/**
 * @brief Deserialization support for simdjson
 *
 */
template <typename simdjson_value, typename Ty>
requires std::is_aggregate_v<Ty>
inline auto tag_invoke(
    simdjson::deserialize_tag, simdjson_value& val, Ty& object
) noexcept // it would return error code
{
    ondemand::object obj;
    auto error = val.get_object().get(obj);
    if (error) [[unlikely]] {
        return error;
    }

    constexpr auto names = ::atom::utils::member_names_of<Ty>();
    [&]<size_t... Is>(::std::index_sequence<Is...>) {
        (::internal::tag_invoke_impl<Is>(obj[names[Is]], ::atom::utils::get<Is>(object)), ...);
    }(::std::make_index_sequence<atom::utils::member_count_v<Ty>>());

    return simdjson::SUCCESS;
}
} // namespace simdjson

    #endif
#endif
