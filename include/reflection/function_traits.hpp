#pragma once
#include <string_view>

namespace atom::utils {

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

struct basic_function_traits {
    explicit constexpr basic_function_traits(const char* name) : name_(name) {}
    explicit constexpr basic_function_traits(std::string_view name) : name_(name) {}

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
struct function_traits<Ret (*)(Args...)> : public ::atom::utils::basic_function_traits {
    using func_type = Ret(Args...);

    explicit constexpr function_traits(const char* name, Ret (*pointer)(Args...))
        : ::atom::utils::basic_function_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr std::size_t num_args() const { return sizeof...(Args); }

    [[nodiscard]] constexpr auto call(Args&&... args) const -> Ret {
        return pointer_(std::forward<Args>(args)...);
    }

    [[nodiscard]] constexpr auto pointer() const noexcept -> Ret (*)(Args...) { return pointer_; }

private:
    Ret (*pointer_)(Args...);
};

template <typename Ret, typename Class, typename... Args>
struct function_traits<Ret (Class::*)(Args...)> : public ::atom::utils::basic_function_traits {
    using func_type = Ret(Args...);

    explicit constexpr function_traits(const char* name, Ret (Class::*pointer)(Args...))
        : ::atom::utils::basic_function_traits(name), pointer_(pointer) {}

    [[nodiscard]] constexpr std::size_t num_args() const { return sizeof...(Args); }

    [[nodiscard]] constexpr auto call(Class& instance, Args&&... args) const -> Ret {
        (instance.*pointer_)(std::forward<Args>(args)...);
    }

    [[nodiscard]] constexpr auto pointer() const noexcept -> Ret (Class::*)(Args...) {
        return pointer_;
    }

private:
    Ret (Class::*pointer_)(Args...);
};

} // namespace atom::utils
