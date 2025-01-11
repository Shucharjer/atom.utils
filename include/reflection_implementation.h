#pragma once
#include <ranges>
#include <shared_mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "initializer.h"
#include "ranges.h"
#include "reflection.h"
#include "tstring.h"
#include "type_traits.h"
#include "./fwd.h"

namespace atom::utils {

struct basic_constexpr_extend {

    /**
     * @class constexpr_extend_info
     * @brief Providing a structure to using designated list initialization, which could omit
     * some operations when defining extended information of the type.
     *
     */
    struct constexpr_extend_info {
        bool is_default_constructible = true;
        bool is_trivial_v             = true;
        bool is_copy_constructible    = true;
        bool is_move_constructible    = true;
        bool is_copy_assignable       = true;
        bool is_move_assignable       = true;
        bool is_destructible          = true;
        bool is_aggregate_v           = false;
        bool is_enum                  = false;
        bool is_component             = false;
        bool is_resource              = false;
    };

    constexpr_extend_info info;

    constexpr basic_constexpr_extend()                                         = default;
    constexpr basic_constexpr_extend(const basic_constexpr_extend&)            = default;
    constexpr basic_constexpr_extend(basic_constexpr_extend&&)                 = default;
    constexpr basic_constexpr_extend& operator=(const basic_constexpr_extend&) = default;
    constexpr basic_constexpr_extend& operator=(basic_constexpr_extend&&)      = default;
    constexpr virtual ~basic_constexpr_extend()                                = default;

    constexpr basic_constexpr_extend(const constexpr_extend_info& info) : info(info) {}
};

template <UCONCEPTS pure Ty>
struct constexpr_extend : public UTILS basic_constexpr_extend {
    constexpr constexpr_extend()
        : UTILS basic_constexpr_extend{
              { .is_default_constructible = std::is_default_constructible_v<Ty>,
               .is_trivial_v             = std::is_trivial_v<Ty>,
               .is_copy_constructible    = std::is_copy_constructible_v<Ty>,
               .is_move_constructible    = std::is_move_constructible_v<Ty>,
               .is_copy_assignable       = std::is_copy_assignable_v<Ty>,
               .is_move_assignable       = std::is_move_assignable_v<Ty>,
               .is_destructible          = std::is_destructible_v<Ty>,
               .is_aggregate_v           = std::is_aggregate_v<Ty>,
               .is_enum                  = std::is_enum_v<Ty> }
    } {}
    constexpr constexpr_extend(const constexpr_extend&)            = default;
    constexpr constexpr_extend(constexpr_extend&&)                 = default;
    constexpr constexpr_extend& operator=(const constexpr_extend&) = default;
    constexpr constexpr_extend& operator=(constexpr_extend&&)      = default;
    constexpr ~constexpr_extend() override                         = default;
};

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
    }

    constexpr virtual ~basic_reflected() = default;

    [[nodiscard]] constexpr auto name() const -> std::string_view { return name_; }

    [[nodiscard]] constexpr auto hash() const -> std::size_t { return hash_; }

    [[nodiscard]] constexpr auto cextend() const -> const basic_constexpr_extend& {
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

class reflection {
public:
    reflection() = delete;

    struct type_pair {
        default_id_t ident;
        std::shared_ptr<basic_reflected<>> reflected;
    };

    static auto hash(std::string_view name) noexcept {
        // typeid(xx_namespace::xx_type) : struct xx_namespace::xx_type
        // so, ...
        std::string string = name.data();

        if (auto found = string.find_last_of(' ');
            found != std::string::npos && found < string.length()) {
            string = string.substr(found + 1);
        }

        return std::hash<std::string_view>()(string);
    }

    static auto get(std::size_t hash) -> const type_pair* {
        auto& mutex_            = get_mutex();
        auto& registered_types_ = get_registered();
        std::shared_lock<std::shared_mutex> slock(mutex_);
        return registered_types_.contains(hash) ? &registered_types_.at(hash) : nullptr;
    }

    static auto get_by_name(std::string_view name) -> const type_pair* {
        auto hash_ = hash(name);

        auto& mutex_            = get_mutex();
        auto& registered_types_ = get_registered();
        std::shared_lock<std::shared_mutex> slock(mutex_);
        return registered_types_.contains(hash_) ? &registered_types_.at(hash_) : nullptr;
    }

    template <typename Ty>
    static auto is_registered() noexcept -> bool {
        auto reflected_ = reflected<Ty>();
        return is_registered(reflected_.name());
    }

    static auto is_registered(std::string_view name) -> bool {
        const auto hash_ = hash(name);
        return is_registered(hash_);
    }

    static auto is_registered(const size_t hash) -> bool {
        auto& mutex_            = get_mutex();
        auto& registered_types_ = get_registered();
        std::shared_lock<std::shared_mutex> slock(mutex_);
        auto registered = registered_types_.contains(hash);
        slock.unlock();
        return registered;
    }

    template <typename Ty>
    static void register_type() {
        using storage_type = ::atom::utils::initializer<Ty, true, ::atom::utils::lazy>;

        auto reflected_ = reflected<Ty>();
        auto& extend_   = reflected_.extend();
        if (reflected_.cextend().info.is_default_constructible) {
            extend_->new_object = []() -> void* { return static_cast<void*>(::new Ty); };
        }

        extend_->delete_object = [](void* ptr) -> void { return static_cast<Ty*>(ptr)->~Ty(); };

        auto hash_              = hash(reflected_.name());
        auto& mutex_            = get_mutex();
        auto& registered_types_ = get_registered();
        std::shared_lock<std::shared_mutex> slock(mutex_);
        auto registered = registered_types_.contains(hash_);
        slock.unlock();
        if (!registered) {
            auto& guid = current_guid();
            type_pair pair{ guid.load(std::memory_order_acquire),
                            std::make_shared<reflected<Ty>>() };
            guid.fetch_add(1, std::memory_order_release);

            std::unique_lock<std::shared_mutex> ulock(mutex_);
            registered_types_.emplace(hash_, std::move(pair));
            ulock.unlock();
        }
    }

    static uint32_t total() noexcept { return current_guid().load(std::memory_order_relaxed); }

    static auto components() noexcept -> std::vector<type_pair> {
        auto& registered_types_ = get_registered();
        auto& mutex_            = get_mutex();

        auto filter = [](const auto& pair) { return pair.reflected->cextend().info.is_component; };
        std::shared_lock<std::shared_mutex> slock(mutex_);
        auto filtered = registered_types_ | std::views::values | std::views::filter(filter);
        slock.unlock();

        return URANGES to<std::vector>(filtered);
    }

    static auto resources() noexcept -> std::vector<type_pair> {
        auto& registered_types_ = get_registered();
        auto& mutex_            = get_mutex();

        auto filter = [](const auto& pair) { return pair.reflected->cextend().info.is_resource; };
        std::shared_lock<std::shared_mutex> slock(mutex_);
        auto filtered = registered_types_ | std::views::values | std::views::filter(filter);
        slock.unlock();

        return URANGES to<std::vector>(filtered);
    }

    static auto all() noexcept -> std::vector<type_pair> {
        return URANGES to<std::vector>(get_registered() | std::views::values);
    }

private:
    inline static auto get_mutex() -> std::shared_mutex& {
        static auto mutex_ = std::shared_mutex{};
        return mutex_;
    }

    inline static auto get_registered() -> std::unordered_map<std::size_t, type_pair>& {
        static auto registered_types_ = std::unordered_map<std::size_t, type_pair>();
        return registered_types_;
    }

    inline static auto current_guid() -> std::atomic<uint32_t>& {
        static auto current_guid_ = std::atomic<uint32_t>{};
        return current_guid_;
    }
};

namespace internal {

template <typename Ty>
class type_register {
public:
    type_register() {
        // register type to reflection
        reflection::register_type<Ty>();
    }
    type_register(const type_register&)            = delete;
    type_register(type_register&&)                 = delete;
    type_register& operator=(const type_register&) = delete;
    type_register& operator=(type_register&&)      = delete;
    ~type_register()                               = default;
};

} // namespace internal

} // namespace atom::utils

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

// This macro must be used before BEGIN_TYPE()
#define CONSTEXPR_EXTEND_INFO(type_name, ...)                                                      \
    template <>                                                                                    \
    struct ::atom::utils::constexpr_extend<type_name>                                              \
        : public ::atom::utils::basic_constexpr_extend {                                           \
        using type                                                     = type_name;                \
        constexpr constexpr_extend(const constexpr_extend&)            = default;                  \
        constexpr constexpr_extend(constexpr_extend&&)                 = default;                  \
        constexpr constexpr_extend& operator=(const constexpr_extend&) = default;                  \
        constexpr constexpr_extend& operator=(constexpr_extend&&)      = default;                  \
        constexpr ~constexpr_extend() override                         = default;                  \
                                                                                                   \
        constexpr constexpr_extend()                                                               \
            : ::atom::utils::basic_constexpr_extend(                                               \
                  { .is_default_constructible = std::is_default_constructible_v<type>,             \
                    .is_trivial_v             = std::is_trivial_v<type>,                           \
                    .is_copy_constructible    = std::is_copy_constructible_v<type>,                \
                    .is_move_constructible    = std::is_move_constructible_v<type>,                \
                    .is_copy_assignable       = std::is_copy_assignable_v<type>,                   \
                    .is_move_assignable       = std::is_move_assignable_v<type>,                   \
                    .is_destructible          = std::is_destructible_v<type>,                      \
                    .is_aggregate_v           = std::is_aggregate_v<type>,                         \
                    .is_enum                  = std::is_enum_v<type>,                              \
                    __VA_ARGS__ }                                                                  \
              ) {}                                                                                 \
    };                                                                                             \
    //

#define IS_COMPONENT(boolean) .is_component = boolean

#define IS_RESOURCE(boolean) .is_resource = boolean

// TODO: generate the hash_code for each type.
#define BEGIN_TYPE(type_name)                                                                      \
    template <>                                                                                    \
    struct ::atom::utils::reflected<                                                               \
        type_name,                                                                                 \
        ::atom::utils::basic_constexpr_extend,                                                     \
        ::atom::utils::constexpr_extend>                                                           \
        final : public ::atom::utils::basic_reflected<::atom::utils::basic_constexpr_extend> {     \
        using type = type_name;                                                                    \
                                                                                                   \
        constexpr reflected()                                                                      \
            : ::atom::utils::basic_reflected<::atom::utils::basic_constexpr_extend>(               \
                  #type_name, std::hash<std::string_view>()(#type_name)                            \
              ) {}                                                                                 \
        //

#define FIELDS(...)                                                                                \
    inline constexpr const auto& fields() const { return fields_; }                                \
    inline constexpr static auto fields_ = std::make_tuple(__VA_ARGS__);                           \
    //

#define FIELD(fieldname)                                                                           \
    ::atom::utils::field_traits<decltype(&type::fieldname)> { #fieldname, &type::fieldname }       \
    //

#define FUNCS(...)                                                                                 \
    inline constexpr const auto& functions() const { return functions_; }                          \
    inline constexpr static auto functions_ = std::make_tuple(__VA_ARGS__);                        \
    //

#define FUNC(funcname)                                                                             \
    ::atom::utils::function_traits<decltype(&type::funcname)> { #funcname, &type::funcname }       \
    //

#define END_TYPE(type_name, register_name)                                                         \
private:                                                                                           \
    constexpr static auto constexpr_extend_ = ::atom::utils::constexpr_extend<type>();             \
                                                                                                   \
    constexpr auto get_constexpr_extend() const                                                    \
        -> const ::atom::utils::basic_constexpr_extend& override {                                 \
        return constexpr_extend_;                                                                  \
    }                                                                                              \
    }                                                                                              \
    ;                                                                                              \
    namespace internal {                                                                           \
    static inline const auto(register_name) = ::atom::utils::internal::type_register<type_name>{}; \
    }                                                                                              \
    //

// NOLINTEND(cppcoreguidelines-macro-usage)
CONSTEXPR_EXTEND_INFO(bool)
BEGIN_TYPE(bool)
FIELDS()
END_TYPE(bool, bool_register)

CONSTEXPR_EXTEND_INFO(char)
BEGIN_TYPE(char)
FIELDS()
END_TYPE(char, char_register)

CONSTEXPR_EXTEND_INFO(wchar_t)
BEGIN_TYPE(wchar_t)
FIELDS()
END_TYPE(wchar_t, wchar_t_register)

CONSTEXPR_EXTEND_INFO(uint8_t)
BEGIN_TYPE(uint8_t)
FIELDS()
END_TYPE(uint8_t, uint8_t_register)

CONSTEXPR_EXTEND_INFO(int8_t)
BEGIN_TYPE(int8_t)
FIELDS()
END_TYPE(int8_t, int8_t_register)

CONSTEXPR_EXTEND_INFO(uint16_t)
BEGIN_TYPE(uint16_t)
FIELDS()
END_TYPE(uint16_t, uint16_t_register)

CONSTEXPR_EXTEND_INFO(int16_t)
BEGIN_TYPE(int16_t)
FIELDS()
END_TYPE(int16_t, int16_t_register)

CONSTEXPR_EXTEND_INFO(uint32_t)
BEGIN_TYPE(uint32_t)
FIELDS()
END_TYPE(uint32_t, uint32_t_register)

CONSTEXPR_EXTEND_INFO(int32_t)
BEGIN_TYPE(int32_t)
FIELDS()
END_TYPE(int32_t, int32_t_register)

CONSTEXPR_EXTEND_INFO(uint64_t)
BEGIN_TYPE(uint64_t)
FIELDS()
END_TYPE(uint64_t, uint64_t_register)

CONSTEXPR_EXTEND_INFO(int64_t)
BEGIN_TYPE(int64_t)
FIELDS()
END_TYPE(int64_t, int64_t_register)

CONSTEXPR_EXTEND_INFO(float)
BEGIN_TYPE(float)
FIELDS()
END_TYPE(float, float_register)

CONSTEXPR_EXTEND_INFO(double)
BEGIN_TYPE(double)
FIELDS()
END_TYPE(double, double_register)

CONSTEXPR_EXTEND_INFO(std::string)
BEGIN_TYPE(std::string)
FIELDS()
END_TYPE(std::string, std_string_register)

#define FCONCEPTS ::atom::ecs::concepts::

namespace atom::ecs::concepts {

template <typename Ty>
concept component = requires {
    ::atom::utils::reflected<Ty>().cextend().info.is_component;
    std::is_default_constructible_v<Ty>;
    std::is_destructible_v<Ty>;
};

template <typename Ty>
concept resource = requires {
    ::atom::utils::reflected<Ty>().cextend().info.is_resource;
    std::is_default_constructible_v<Ty>;
    std::is_destructible_v<Ty>;
};

} // namespace atom::ecs::concepts

// Serialization

#if __has_include(<nlohmann/json.hpp>)
    #include <nlohmann/json.hpp>

namespace internal::reflection {

template <std::size_t Index, typename Ty>
void to_json_impl_modify(nlohmann::json& json, const Ty& obj) {
    json[std::get<Index>(UTILS reflected<Ty>::fields_).name()] =
        std::get<Index>(UTILS reflected<Ty>::fields_).get(obj);
}

template <typename Ty, std::size_t... Is>
void to_json_impl(nlohmann::json& json, const Ty& obj, std::index_sequence<Is...>) {
    (to_json_impl_modify<Is>(json, obj), ...);
}

template <std::size_t Index, typename Ty>
void from_json_impl_modify(const nlohmann::json& json, Ty& obj) {
    json.at(std::get<Index>(UTILS reflected<Ty>::fields_).name())
        .get_to(std::get<Index>(UTILS reflected<Ty>::fields_).get(obj));
}

template <typename Ty, std::size_t... Is>
void from_json_impl(const nlohmann::json& json, Ty& obj, std::index_sequence<Is...>) {
    auto impl = [&json, &obj]<std::size_t Index>() {};
    (from_json_impl_modify<Is>(json, obj), ...);
}

} // namespace internal::reflection

template <typename Ty>
void to_json(nlohmann::json& json, const Ty& obj) {
    using pure_type = std::remove_cvref_t<Ty>;
    internal::reflection::to_json_impl<Ty>(
        json,
        obj,
        std::make_index_sequence<UTILS tuple_size_v<decltype(UTILS reflected<pure_type>::fields_)>>(
        )
    );
}

template <typename Ty>
void from_json(const nlohmann::json& json, Ty& obj) {
    using pure_type = std::remove_cvref_t<Ty>;
    internal::reflection::from_json_impl(
        json,
        obj,
        std::make_index_sequence<UTILS tuple_size_v<decltype(UTILS reflected<pure_type>::fields_)>>(
        )
    );
}

#endif
