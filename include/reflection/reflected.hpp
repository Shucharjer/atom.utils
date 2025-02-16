#pragma once
#include <string_view>
#include "concepts/type.hpp"
#include "description.hpp"
#include "reflection/hash.hpp"
#include "structures/tstring.hpp"

namespace atom::utils {

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
constexpr static inline std::string_view empty_string = "";
constexpr static inline std::tuple<> empty_tuple{};
} // namespace internal
/*! @endcond */

/**
 * @brief Recording basic information about a type.
 *
 */
struct basic_reflected {
public:
    constexpr basic_reflected(const basic_reflected&)            = default;
    constexpr basic_reflected& operator=(const basic_reflected&) = default;

    constexpr basic_reflected(basic_reflected&& obj) noexcept
        : name_(obj.name_), hash_(obj.hash_), description_(obj.description_) {}
    constexpr basic_reflected& operator=(basic_reflected&& obj) noexcept {
        name_        = obj.name_;
        hash_        = obj.hash_;
        description_ = obj.description_;
        return *this;
    }

    constexpr ~basic_reflected() = default;

    /**
     * @brief Get the name of this reflected type.
     *
     * @return std::string_view The name.
     */
    [[nodiscard]] constexpr auto name() const -> std::string_view { return name_; }

    /**
     * @brief Get the hash value of this reflected type.
     *
     * @return std::size_t
     */
    [[nodiscard]] constexpr auto hash() const -> std::size_t { return hash_; }

    [[nodiscard]] constexpr auto description() const -> description_bits { return description_; }

protected:
    constexpr explicit basic_reflected(
        const char* const name, const std::size_t hash, description_bits description)
        : name_(name), hash_(hash), description_(description) {}

private:
    const char* name_;
    std::size_t hash_;
    description_bits description_;
};

/**
 * @brief A type stores reflected information.
 *
 * @tparam Ty The reflected type.
 * @tparam BaseConstexprExtend A type contains basic compile time extend information.
 * @tparam ConstexprExtend A type contains advanced compile time extend information.
 */
template <concepts::pure Ty>
struct reflected final : public ::atom::utils::basic_reflected {
    constexpr reflected() noexcept
        : ::atom::utils::basic_reflected(name_of<Ty>(), hash_of<Ty>(), description_of<Ty>) {}

    reflected(const reflected&) noexcept            = default;
    reflected(reflected&&) noexcept                 = default;
    reflected& operator=(const reflected&) noexcept = default;
    reflected& operator=(reflected&&) noexcept      = default;
    constexpr ~reflected() noexcept                 = default;

    /**
     * @brief Fields exploded to outside in the type.
     *
     * @return constexpr const auto& A tuple contains function traits.
     */
    constexpr const auto& fields() const {
        if constexpr (concepts::aggregate<Ty>) {}
        else if constexpr (concepts::has_field_traits<Ty>) {
            return Ty::field_traits();
        }
        else {
            return internal::empty_tuple;
        }
    }

    /**
     * @brief Functions exploded to outside in the type.
     *
     * @return constexpr const auto& A tuple contains function traits.
     */
    constexpr const auto& functions() const {
        if constexpr (concepts::has_function_traits<Ty>) {
            return Ty::function_traits();
        }
        else {
            return internal::empty_tuple;
        }
    }
};

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {
template <::atom::utils::tstring_v Name, std::size_t Index, typename Tuple>
constexpr void find_traits(const Tuple& tuple, std::size_t& result) {
    if (std::get<Index>(tuple).name() == Name.val) {
        result = Index;
    }
}
} // namespace internal
/*! @endcond */

template <::atom::utils::tstring_v Name, typename Tuple>
consteval std::size_t index_of(const Tuple& tuple) {
    auto index_sequence = std::make_index_sequence<::atom::utils::tuple_size_v<Tuple>>();
    std::size_t result  = ::atom::utils::tuple_size_v<Tuple>;
    []<std::size_t... Is>(const Tuple& tuple, std::size_t& result, std::index_sequence<Is...>) {
        (internal::find_traits<Name, Is>(tuple, result), ...);
    }(tuple, result, index_sequence);
    return result;
}

// Sadly, this is not supported.
// We could only do this:
// constexpr auto index = index_of<"field name">(tuple);
// auto& traits = std::get<index>(tuple);
//
// template <::atom::utils:: tstring_v Name, typename Tuple>
// constexpr auto traits_of(const Tuple& tuple) {
//     constexpr std::size_t index = index_of<Name>(tuple);
//     static_assert(index < tuple_size_v<Tuple>, "This tuple doesn't have so many elements.");
//     return std::get<index>(tuple);
// }

} // namespace atom::utils

#if __has_include(<lua.hpp>) && __has_include(<sol/sol.hpp>)
    #include <sol/sol.hpp>

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

template <size_t Index, typename Ty, typename Tuple>
inline void bind_fields_to_lua_impl(const Tuple& fields, sol::usertype<Ty>& usertype) noexcept {
    const auto& traits      = std::get<Index>(fields);
    usertype[traits.name()] = traits.pointer();
}

template <typename Ty, typename Tuple, size_t... Is>
inline void bind_fields_to_lua(
    const Tuple& fields, sol::usertype<Ty>& usertype, std::index_sequence<Is...>) noexcept {
    (bind_fields_to_lua_impl<Is>(fields, usertype), ...);
}

template <size_t Index, typename Ty, typename Tuple>
inline void bind_functions_to_lua_impl(
    const Tuple& functions, sol::usertype<Ty>& usertype) noexcept {
    const auto& traits      = std::get<Index>(functions);
    usertype[traits.name()] = traits.pointer();
}

template <typename Ty, typename Tuple, size_t... Is>
inline void bind_functions_to_lua(
    const Tuple& functions, sol::usertype<Ty>& usertype, std::index_sequence<Is...>) noexcept {
    (bind_functions_to_lua_impl<Is>(functions, usertype), ...);
}

} // namespace internal
/*! @endcond */

/**
 * @brief Bind a usertype to lua state.
 *
 */
template <typename Ty>
inline sol::usertype<Ty> bind_to_lua(sol::state& lua) noexcept {
    auto reflected = ::atom::utils::reflected<Ty>();

    auto usertype = lua.new_usertype<Ty>(reflected.name());
    if constexpr (std::is_default_constructible_v<Ty>) {
        usertype["new"] = sol::constructors<Ty()>();
    }

    if constexpr (requires {
                      ::atom::utils::reflected<Ty>();
                      ::atom::utils::reflected<Ty>().fields();
                  }) {
    #if __has_include(<nlohmann/json.hpp>)
        usertype["to_json"]   = &to_new_json<Ty>;
        usertype["from_json"] = &set_from_json<Ty>;
    #endif

        const auto& fields = reflected.fields();
        internal::bind_fields_to_lua<Ty>(
            fields, usertype,
            std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(fields)>>>());
    }

    if constexpr (requires {
                      ::atom::utils::reflected<Ty>();
                      ::atom::utils::reflected<Ty>().funcs();
                  }) {
        const auto& functions = reflected.funcs();
        internal::bind_functions_to_lua<Ty>(
            functions, usertype,
            std::make_index_sequence<
                std::tuple_size_v<std::remove_cvref_t<decltype(functions)>>>());
    }

    return usertype;
}

template <typename Ty>
inline auto wrapped_bind_to_lua() -> void (*)(sol::state&) {
    return [](sol::state& lua) -> void { std::ignore = bind_to_lua<Ty>(lua); };
}

#endif
