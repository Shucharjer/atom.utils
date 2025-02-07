#pragma once
#include <type_traits>
#include "concepts/type.hpp"
#include "reflection.hpp"
#include "reflection/constexpr_extend.hpp"
#include "reflection/extend.hpp"
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
 * @tparam BasicConstexprExtend Basic compile time information.
 */
template <::atom::utils::concepts::pure BasicConstexprExtend>
struct basic_reflected {
public:
    constexpr explicit basic_reflected(const char* const name)
        : basic_reflected(name, utils::hash(name)) {}

    constexpr explicit basic_reflected(std::string_view name)
        : basic_reflected(name.data(), hash) {}

    constexpr basic_reflected() : basic_reflected(internal::empty_string.data()) {}

    template <typename TString>
    requires std::is_base_of_v<::atom::utils::basic_tstring, TString>
    constexpr explicit basic_reflected(std::size_t hash)
        : name_(std::make_shared<TString>()), hash_(hash) {}

    constexpr basic_reflected(const basic_reflected&)            = default;
    constexpr basic_reflected& operator=(const basic_reflected&) = default;

    constexpr basic_reflected(basic_reflected&& obj) noexcept
        : name_(obj.name_), hash_(obj.hash_) {}
    constexpr basic_reflected& operator=(basic_reflected&& obj) noexcept {
        name_ = obj.name_;
        hash_ = obj.hash_;
        return *this;
    }

    constexpr virtual ~basic_reflected() = default;

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

    /**
     * @brief Get the compile time extend information.
     *
     * @return const BasicConstexprExtend&
     */
    [[nodiscard]] constexpr auto cextend() const -> const BasicConstexprExtend& {
        return get_constexpr_extend();
    }

    /**
     * @brief Get the runtime extend information.
     *
     * @return auto&
     */
    [[nodiscard]] auto& extend() { return get_extend(); }
    /**
     * @brief
     *
     * @return const auto&
     */
    [[nodiscard]] const auto& extend() const { return get_extend(); }

protected:
    static void relocate(basic_reflected& reflected, const char* name) {
        reflected.name_ = name;
        reflected.hash_ = utils::hash(name);
    }

    constexpr explicit basic_reflected(const char* const name, const std::size_t hash)
        : name_(name), hash_(hash) {}

private:
    const char* name_;
    std::size_t hash_;

    constexpr static auto default_constexpr_extend_ = BasicConstexprExtend();

    [[nodiscard]] virtual auto get_constexpr_extend() const -> const BasicConstexprExtend& {
        return default_constexpr_extend_;
    }

    /**
     * @brief Get the static instance contains extend information.
     * Cheshire Cat.
     *
     * @return The shared pointer of extend.
     */
    static auto get_extend() -> std::shared_ptr<struct extend>& {
        // Cheshire Cat Idioms
        static std::shared_ptr<struct extend> extend_ = std::make_shared<struct extend>();
        return extend_;
    }
};

/**
 * @brief A type stores reflected information.
 *
 * @tparam Ty The reflected type.
 * @tparam BaseConstexprExtend A type contains basic compile time extend information.
 * @tparam ConstexprExtend A type contains advanced compile time extend information.
 */
template <
    ::atom::utils::concepts::pure Ty,
    ::atom::utils::concepts::pure BasicConstexprExtend,
    template <::atom::utils::concepts::pure> typename ConstexprExtend>
struct reflected final : public ::atom::utils::basic_reflected<BasicConstexprExtend> {
    using type = Ty;
    constexpr reflected() : ::atom::utils::basic_reflected<BasicConstexprExtend>() {
        auto name = get_name();
        basic_reflected<BasicConstexprExtend>::relocate(
            static_cast<basic_reflected<BasicConstexprExtend>&>(*this), name
        );
    }

    /**
     * @brief Fields exploded to outside in the type.
     *
     * @return constexpr const auto& A tuple contains function traits.
     */
    constexpr const auto& fields() const {
        if constexpr (requires(const Ty& obj) { Ty::reflect(static_cast<Ty*>(nullptr)); } &&
                      std::is_default_constructible_v<Ty>) {
            return Ty{}.reflect(static_cast<const Ty*>(nullptr));
        }
        else if constexpr (requires { reflect(static_cast<Ty*>(nullptr)); }) {
            return reflect(static_cast<const Ty*>(nullptr));
        }
        else if constexpr (requires(const Ty& obj) { obj.reflect(); } &&
                           std::is_default_constructible_v<Ty>) {
            return Ty{}.reflect();
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
    constexpr const auto& functions() const { return internal::empty_tuple; }

private:
    constexpr static auto constexpr_extend_ = ::atom::utils::constexpr_extend<type>();

    static inline const char* get_name() {
        static std::string name = internal::process_name(typeid(Ty).name());
        return name.data();
    }

    [[nodiscard]] constexpr auto get_constexpr_extend() const
        -> const ::atom::utils::basic_constexpr_extend& override {
        return constexpr_extend_;
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

#if __has_include(<nlohmann/json.hpp>)
    #include <nlohmann/json.hpp>
    #include "core/type_traits.hpp"

/*! @cond TURN_OFF_DOXYGEN */
namespace internal::reflection {

template <std::size_t Index, typename Ty, typename Tuple>
constexpr void to_json_impl(nlohmann::json& json, const Ty& obj, const Tuple& tuple) {
    json[std::get<Index>(tuple).name()] = std::get<Index>(tuple).get(obj);
}

template <typename Ty, typename Tuple, std::size_t... Is>
constexpr void
    to_json(nlohmann::json& json, const Ty& obj, const Tuple& tuple, std::index_sequence<Is...>) {
    (to_json_impl<Is>(json, obj, tuple), ...);
}

template <std::size_t Index, typename Ty, typename Tuple>
constexpr void from_json_impl(const nlohmann::json& json, Ty& obj, const Tuple& tuple) {
    json.at(std::get<Index>(tuple).name()).get_to(std::get<Index>(tuple).get(obj));
}

template <typename Ty, typename Tuple, std::size_t... Is>
constexpr void
    from_json(const nlohmann::json& json, Ty& obj, const Tuple& tuple, std::index_sequence<Is...>) {
    (from_json_impl<Is>(json, obj, tuple), ...);
}

} // namespace internal::reflection
/*! @endcond */

/**
 * @brief Store an instance's information into a `nlohmann::json` object.
 *
 */
template <typename Ty>
constexpr void to_json(nlohmann::json& json, const Ty& obj) {
    using pure_t       = std::remove_cvref_t<Ty>;
    const auto& fields = ::atom::utils::reflected<pure_t>().fields();
    internal::reflection::to_json<Ty>(
        json, obj, fields, std::make_index_sequence<::atom::utils::tuple_size_v<decltype(fields)>>()
    );
}

/**
 * @brief Restore an instance's information from a `nlohmann::json` object.
 *
 */
template <typename Ty>
constexpr void from_json(const nlohmann::json& json, Ty& obj) {
    using pure_t       = std::remove_cvref_t<Ty>;
    const auto& fields = ::atom::utils::reflected<pure_t>().fields();
    internal::reflection::from_json(
        json, obj, fields, std::make_index_sequence<::atom::utils::tuple_size_v<decltype(fields)>>()
    );
}

#endif

#if __has_include(<simdjson.h>)
    #include <simdjson.h>

/*! @cond TURN_OFF_DOXYGEN */
namespace internal::reflection {
template <size_t Index, typename Ty, typename Tuple>
auto tag_invoke_impl(simdjson::ondemand::object& obj, Ty& object, const Tuple& fields) noexcept {
    const auto& traits = std::get<Index>(fields);
    auto inst          = obj[traits.name()];
    auto& elem         = traits.get(object);
    using elem_t       = std::remove_cvref_t<decltype(elem)>;
    if constexpr (std::is_same_v<elem_t, bool>) {
        return inst.get_bool(elem);
    }
    else if constexpr (std::is_same_v<elem_t, uint64_t>) {
        return inst.get_uint64(elem);
    }
    else if constexpr (std::is_same_v<elem_t, int64_t>) {
        return inst.get_int64(elem);
    }
    else if constexpr (std::is_same_v<elem_t, double>) {
        return inst.get_double(elem);
    }
    else if constexpr (std::is_same_v<elem_t, std::string_view>) {
        return inst.get_string(elem);
    }
    else if constexpr (std::is_same_v<elem_t, simdjson::ondemand::object>) {
        return inst.get_object(elem);
    }
    else if constexpr (std::is_same_v<elem_t, simdjson::ondemand::array>) {
        return inst.get_array(elem);
    }
    else {
        return inst.get(elem);
    }
}

template <typename Ty, typename Tuple, size_t... Is>
auto tag_invoke(simdjson::ondemand::object& obj, Ty& object, const Tuple& fields, std::index_sequence<Is...>) noexcept {
    return (tag_invoke_impl<Is>(obj, object, fields), ...);
}
} // namespace internal::reflection

namespace simdjson {
template <typename simdjson_value, typename Ty>
requires(
    std::tuple_size_v<std::remove_cvref_t<decltype(::atom::utils::reflected<Ty>().fields())>> != 0
)
auto tag_invoke(simdjson::deserialize_tag, simdjson_value& val, Ty& object) noexcept {
    ondemand::object obj;
    auto error = val.get_object().get(obj);
    if (error) [[unlikely]] {
        return error;
    }

    using pure_t       = std::remove_cvref_t<Ty>;
    const auto& fields = ::atom::utils::reflected<pure_t>().fields();
    if (error = ::internal::reflection::tag_invoke(
            obj,
            object,
            fields,
            std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<decltype(fields)>>>()
        );
        error) [[unlikely]] {
        return error;
    }

    return simdjson::SUCCESS;
}
} // namespace simdjson

/*! @endcond */
#endif
