/**
 * @file reflection.h
 * @author Shucharjer (2592284636@qq.com)
 * @brief This file contains some declarations and definitions about reflection.
 * @version 0.1
 * @date 2024-12-07
 *
 * This header file only provides an extensible reflection interface.
 * The specific implementation needs to be completed according to the specific situation.
 *
 * @copyright Copyright (c) 2024
 *
 *
 */

#pragma once

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

/**
 * @brief Base of class template function_traits.
 *
 */
struct basic_function_traits;

/**
 * @brief Traits of function.
 */
template <typename>
struct function_traits;

/**
 * @class basic_constexpr_extend
 * @brief Extend informations about a type, which could be determined at compile time.
 *
 */
struct basic_constexpr_extend;

/**
 * @brief Extend informations about a type, which could be determined at compile time.
 */
template <UCONCEPTS pure>
struct constexpr_extend;

/**
 * @class extend
 * @brief Extend informations about a type, which could only be determined at runtime.
 *
 */
struct extend;

/**
 * @brief Base of class template `reflected`.
 *
 * This could provide a same base to collect reflected informations to a contain easily.
 */
template <UCONCEPTS pure BaseConstexprExtend = basic_constexpr_extend>
struct basic_reflected;

/**
 * @brief Reflected informations of a type.
 *
 * @tparam Ty Reflected type.
 */
template <
    UCONCEPTS pure Ty,
    UCONCEPTS pure BaseConstexprExtend                 = basic_constexpr_extend,
    template <UCONCEPTS pure> typename ConstexprExtend = constexpr_extend>
struct reflected;

class reflection;

} // namespace atom::utils

struct UTILS basic_field_traits {
    explicit constexpr basic_field_traits(const char* name) : name_(name) {}
    explicit constexpr basic_field_traits(std::string_view name) : name_(name) {}
    explicit constexpr basic_field_traits(const std::string& name) : name_(name) {}

    constexpr basic_field_traits(const basic_field_traits&)            = default;
    constexpr basic_field_traits& operator=(const basic_field_traits&) = default;

    constexpr basic_field_traits(basic_field_traits&& obj) noexcept : name_(obj.name_) {}
    constexpr basic_field_traits& operator=(basic_field_traits&& obj) noexcept {
        if (this != &obj) {
            name_ = obj.name_;
        }
        return *this;
    }

    constexpr virtual ~basic_field_traits() = default;

    [[nodiscard]] constexpr auto name() const -> std::string_view { return name_; }

private:
    std::string_view name_;
};

template <>
struct UTILS field_traits<void> : public UTILS basic_field_traits {
    using type = void;

    explicit constexpr field_traits() : UTILS basic_field_traits("void") {}
};

template <typename Ty>
requires(!std::is_void_v<Ty>)
struct UTILS field_traits<Ty*> : public UTILS basic_field_traits {
    using type = Ty;

    explicit constexpr field_traits(const char* name, Ty* pointer)
        : ::atom::utils::basic_field_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr auto get() -> Ty& { return *pointer_; }
    [[nodiscard]] constexpr auto get() const -> const Ty& { return *pointer_; }

private:
    Ty* pointer_;
};

template <typename Ty, typename Class>
struct UTILS field_traits<Ty Class::*> : public UTILS basic_field_traits {
    using type = Ty;

    explicit constexpr field_traits(const char* name, Ty Class::* pointer)
        : UTILS basic_field_traits(name), pointer_(pointer) {
        // set_name(name_);
    }

    [[nodiscard]] constexpr auto get(Class& instance) const -> Ty& { return instance.*pointer_; }
    [[nodiscard]] constexpr auto get(const Class& instance) const -> const Ty& {
        return instance.*pointer_;
    }

private:
    Ty Class::* pointer_;
};

struct ::atom::utils::basic_function_traits {
    explicit constexpr basic_function_traits(const char* name) : name_(name) {}
    explicit constexpr basic_function_traits(std::string_view name) : name_(name) {}
    explicit constexpr basic_function_traits(const std::string& name) : name_(name) {}

    constexpr basic_function_traits(const basic_function_traits&)            = default;
    constexpr basic_function_traits& operator=(const basic_function_traits&) = default;

    constexpr basic_function_traits(basic_function_traits&& obj) noexcept : name_(obj.name_) {}
    constexpr basic_function_traits& operator=(basic_function_traits&& obj) noexcept {
        if (this != &obj) {
            name_ = obj.name_;
        }
        return *this;
    }

    constexpr virtual ~basic_function_traits() = default;

    [[nodiscard]] constexpr auto name() const -> std::string_view { return name_; }

private:
    std::string_view name_;
};

template <typename Ret, typename... Args>
struct UTILS function_traits<Ret (*)(Args...)> : public UTILS basic_function_traits {
    using func_type = Ret(Args...);

    explicit constexpr function_traits(const char* name, Ret (*pointer)(Args...))
        : UTILS basic_function_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr std::size_t num_args() const { return sizeof...(Args); }

    [[nodiscard]] constexpr auto call(Args&&... args) const -> Ret {
        return pointer_(std::forward<Args>(args)...);
    }

private:
    Ret (*pointer_)(Args...);
};

template <typename Ret, typename Class, typename... Args>
struct UTILS function_traits<Ret (Class::*)(Args...)> : public UTILS basic_function_traits {
    using func_type = Ret(Args...);

    explicit constexpr function_traits(const char* name, Ret (Class::*pointer)(Args...))
        : UTILS basic_function_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr std::size_t num_args() const { return sizeof...(Args); }

    [[nodiscard]] constexpr auto call(Class& instance, Args&&... args) const -> Ret {
        (instance.*pointer_)(std::forward<Args>(args)...);
    }

private:
    Ret (Class::*pointer_)(Args...);
};
