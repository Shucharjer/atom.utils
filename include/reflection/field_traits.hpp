#pragma once
#include <string_view>

namespace atom::utils {

/**
 * @brief Base of class template field_traits.
 *
 */
struct basic_field_traits;

/**
 * @brief Traits of filed.
 */
template <typename = void>
struct field_traits;

struct basic_field_traits {
    explicit constexpr basic_field_traits(const char* name) : name_(name) {}
    explicit constexpr basic_field_traits(std::string_view name) : name_(name) {}

    constexpr basic_field_traits(const basic_field_traits&)            = default;
    constexpr basic_field_traits& operator=(const basic_field_traits&) = default;

    constexpr basic_field_traits(basic_field_traits&& obj) noexcept : name_(obj.name_) {}
    constexpr basic_field_traits& operator=(basic_field_traits&& obj) noexcept {
        if (this != &obj) {
            name_ = obj.name_;
        }
        return *this;
    }

    constexpr ~basic_field_traits() = default;

    [[nodiscard]] constexpr auto name() const -> std::string_view { return name_; }

private:
    std::string_view name_;
};

template <>
struct field_traits<void> : public ::atom::utils::basic_field_traits {
    using type = void;

    explicit constexpr field_traits() : basic_field_traits("void") {}
};

template <typename Ty>
requires(!std::is_void_v<Ty>)
struct field_traits<Ty*> : public ::atom::utils::basic_field_traits {
    using type = Ty;

    explicit constexpr field_traits(const char* name, Ty* pointer)
        : ::atom::utils::basic_field_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr auto get() -> Ty& { return *pointer_; }
    [[nodiscard]] constexpr auto get() const -> const Ty& { return *pointer_; }

    [[nodiscard]] constexpr auto pointer() const noexcept -> Ty* { return pointer_; }

private:
    Ty* pointer_;
};

template <typename Ty, typename Class>
struct field_traits<Ty Class::*> : public ::atom::utils::basic_field_traits {
    using type = Ty;

    explicit constexpr field_traits(const char* name, Ty Class::* pointer)
        : ::atom::utils::basic_field_traits(name), pointer_(pointer) {
        // set_name(name_);
    }

    [[nodiscard]] constexpr auto get(Class& instance) const -> Ty& { return instance.*pointer_; }
    [[nodiscard]] constexpr auto get(const Class& instance) const -> const Ty& {
        return instance.*pointer_;
    }

    [[nodiscard]] constexpr auto pointer() const noexcept -> Ty Class::* { return pointer_; }

private:
    Ty Class::* pointer_;
};

} // namespace atom::utils
