#pragma once
#include "concepts/type.hpp"
#include "member_count.hpp"
#include "reflection/aggregate/tuple_view.hpp"

namespace atom::utils {

template <size_t Index, concepts::default_reflectible_aggregate Ty>
constexpr inline auto& get(Ty& obj) noexcept {
    static_assert(Index < member_count_v<Ty>);
    auto tuple = internal::object_to_tuple_view(obj);
    return std::get<Index>(tuple);
}

template <size_t Index, concepts::has_field_traits Ty>
constexpr inline auto& get(Ty& obj) noexcept {
    static_assert(Index < member_count_v<Ty>);
    auto tuple = std::remove_cvref_t<Ty>::field_traits();
    return std::get<Index>(tuple).get(obj);
}

} // namespace atom::utils

#if __has_include(<nlohmann/json.hpp>)
    #include <nlohmann/json.hpp>

template <::atom::utils::concepts::reflectible Ty>
constexpr inline void to_json(nlohmann::json& json, const Ty& obj) {
    constexpr auto names = atom::utils::member_names_of<Ty>();
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((json[names[Is]] = ::atom::utils::get<Is>(obj)), ...);
    }(std::make_index_sequence<atom::utils::member_count_v<Ty>>());
}

template <::atom::utils::concepts::reflectible Ty>
constexpr inline void from_json(const nlohmann::json& json, Ty& obj) {
    constexpr auto names = atom::utils::member_names_of<Ty>();
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((json.at(names[Is]).get_to(::atom::utils::get<Is>(obj))), ...);
    }(std::make_index_sequence<atom::utils::member_count_v<Ty>>());
}

#endif

#if !defined(__GNUC__) && __has_include(<simdjson.h>)
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
    else if constexpr (
        std::is_same_v<type, std::string_view> || std::is_same_v<type, std::string>) {
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
template <typename simdjson_value, ::atom::utils::concepts::reflectible Ty>
inline auto tag_invoke(
    simdjson::deserialize_tag, simdjson_value& val,
    Ty& object) noexcept // it would return error code
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

#if __has_include(<lua.hpp>) && __has_include(<sol/sol.hpp>)
    #include <type_traits>
    #include <sol/sol.hpp>
    #include "reflection/name.hpp"

template <::atom::utils::concepts::pure Ty>
inline sol::usertype<Ty> bind_to_lua(sol::state& lua) noexcept {
    constexpr auto name = ::atom::utils::name_of<Ty>();
    auto usertype       = lua.new_usertype<Ty>();

    if constexpr (std::is_default_constructible_v<Ty>) {
        usertype["new"] = sol::constructors<Ty()>();
    }

    if constexpr (::atom::utils::concepts::reflectible<Ty>) {
        // TODO:

    #if __has_include(<nlohmann/json.hpp>)
            // TODO:
    #endif
    }

    if constexpr (::atom::utils::concepts::has_function_traits<Ty>) {
        // TODO:
    }
}

#endif
