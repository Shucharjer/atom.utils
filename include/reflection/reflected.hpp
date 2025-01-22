#pragma once
#include "concepts/type.hpp"
#include "core.hpp"
#include "reflection.hpp"
#include "reflection/constexpr_extend.hpp"
#include "reflection/extend.hpp"
#include "structures/tstring.hpp"

namespace atom::utils {

template <UCONCEPTS pure BasicConstexprExtend>
struct basic_reflected {
public:
    constexpr explicit basic_reflected(const char* name = nullptr, std::size_t hash = 0)
        : name_(name), hash_(hash) {}

    template <typename TString>
    requires std::is_base_of_v<UTILS basic_tstring, TString>
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

    [[nodiscard]] constexpr auto name() const -> std::string_view { return name_; }

    [[nodiscard]] constexpr auto hash() const -> std::size_t { return hash_; }

    [[nodiscard]] constexpr auto cextend() const -> const BasicConstexprExtend& {
        return get_constexpr_extend();
    }

    [[nodiscard]] auto& extend() { return get_extend(); }
    [[nodiscard]] const auto& extend() const { return get_extend(); }

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

template <
    UCONCEPTS pure Ty,
    UCONCEPTS pure BaseConstexprExtend,
    template <UCONCEPTS pure> typename ConstexprExtend>
struct reflected final : public UTILS basic_reflected<UTILS basic_constexpr_extend> {
    using type = Ty;
    constexpr reflected() : UTILS basic_reflected<UTILS basic_constexpr_extend>() {}

    inline constexpr const auto& fields() const { return fields_; }
    inline constexpr const auto& functions() const { return functions_; }

private:
    constexpr static auto constexpr_extend_ = UTILS constexpr_extend<type>();
    constexpr static auto fields_           = std::make_tuple<>();
    constexpr static auto functions_        = std::make_tuple<>();

    [[nodiscard]] constexpr auto get_constexpr_extend() const
        -> const UTILS basic_constexpr_extend& override {
        return constexpr_extend_;
    }
};

namespace internal {
template <UTILS tstring_v Name, std::size_t Index, typename Tuple>
constexpr void find_traits(const Tuple& tuple, std::size_t& result) {
    if (std::get<Index>(tuple).name() == Name.val) {
        result = Index;
    }
}
} // namespace internal

template <UTILS tstring_v Name, typename Tuple>
consteval std::size_t index_of(const Tuple& tuple) {
    auto index_sequence = std::make_index_sequence<UTILS tuple_size_v<Tuple>>();
    std::size_t result  = UTILS tuple_size_v<Tuple>;
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
// template <UTILS tstring_v Name, typename Tuple>
// constexpr auto traits_of(const Tuple& tuple) {
//     constexpr std::size_t index = index_of<Name>(tuple);
//     static_assert(index < tuple_size_v<Tuple>, "This tuple doesn't have so many elements.");
//     return std::get<index>(tuple);
// }

} // namespace atom::utils

#if __has_include(<nlohmann/json.hpp>)
    #include <nlohmann/json.hpp>
    #include "core/type_traits.hpp"

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
    auto impl = [&json, &obj]<std::size_t Index>() {};
    (from_json_impl<Is>(json, obj, tuple), ...);
}

} // namespace internal::reflection

template <typename Ty>
constexpr void to_json(nlohmann::json& json, const Ty& obj) {
    using pure_t       = std::remove_cvref_t<Ty>;
    const auto& fields = ::atom::utils::reflected<pure_t>().fields();
    internal::reflection::to_json<Ty>(
        json, obj, fields, std::make_index_sequence<UTILS tuple_size_v<decltype(fields)>>()
    );
}

template <typename Ty>
constexpr void from_json(const nlohmann::json& json, Ty& obj) {
    using pure_t       = std::remove_cvref_t<Ty>;
    const auto& fields = ::atom::utils::reflected<pure_t>().fields();
    internal::reflection::from_json(
        json, obj, fields, std::make_index_sequence<UTILS tuple_size_v<decltype(fields)>>()
    );
}

#endif
