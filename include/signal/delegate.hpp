#pragma once
#include <type_traits>
#include "core.hpp"
#include "core/type_traits.hpp"
#include "signal.hpp"

namespace atom::utils {

template <typename Ret, typename... Args>
class delegate<Ret(Args...)> final {
public:
    using self_type     = delegate;
    using return_type   = Ret;
    using function_type = Ret(void const*, Args...);
    using type          = Ret(Args...);

    delegate() noexcept : context_(nullptr), function_(nullptr) {}
    delegate(const delegate& other) noexcept = default;
    delegate(delegate&& other) noexcept      = default;
    virtual ~delegate()                      = default;

    template <auto Candidate>
    explicit delegate(utils::spreader<Candidate> spreader) noexcept : context_(nullptr) {
        bind<Candidate>();
    }

    template <auto Candidate, typename Type>
    explicit delegate(utils::spreader<Candidate> spreader, Type& instance) noexcept
        : context_(nullptr) {
        bind<Candidate>(instance);
    }

    explicit delegate(function_type* function, void const* payload) : context_(nullptr) {
        bind(function, payload);
    }

    /**
     * @brief
     *
     * @tparam Candidate
     */
    template <auto Candidate>
    void bind() noexcept {
        static_assert(
            std::is_invocable_r_v<Ret, decltype(Candidate), Args...>,
            "'Candidate' should be a complete function."
        );

        context_  = nullptr;
        function_ = [](void const*, Args... args) -> Ret {
            return Ret(std::invoke(Candidate, std::forward<Args>(args)...));
        };
    }

    /**
     * @brief
     *
     * @tparam Candidate
     * @tparam Type
     * @param instance
     */
    template <auto Candidate, typename Type>
    void bind(Type& instance) noexcept {
        static_assert(
            std::is_invocable_r_v<Ret, decltype(Candidate), Type*, Args...>,
            "'Candidate' should be a complete function."
        );

        context_  = &instance;
        function_ = [](void const* payload, Args... args) -> Ret {
            Type* type_instance =
                static_cast<Type*>(const_cast<utils::same_constness_t<void, Type>*>(payload));
            return Ret(std::invoke(Candidate, *type_instance, std::forward<Args>(args)...));
        };
    }

    /**
     * @brief Bind function to this delegate
     *
     * @param function Pointer to the function
     * @param payload
     */
    void bind(function_type* function, void const* payload) {
        assert(function);
        function_ = function;
        context_  = payload;
    }

    /**
     * @brief Call function has already bind
     *
     * @param args Arguments
     * @return Ret Function's return type
     */
    Ret operator()(Args... args) const { return function_(context_, std::forward<Args>(args)...); }

    operator bool() const noexcept { return function_; }

    delegate& operator=(const delegate& other) noexcept {
        // type of `function_` and `context_` are pointer
        // do not check self assignment
        function_ = other.function_;
        context_  = other.context_;
        return *this;
    }

    delegate& operator=(delegate&& other) noexcept {
        // copy pointer, no check
        function_ = other.function_;
        context_  = other.context_;

        return *this;
    }

    template <typename Type>
    bool operator==(delegate<Type>& other) const noexcept {
        if constexpr (std::is_same_v<Ret(Args...), Type>) {
            return function_ == other._function && context_ == other._context;
        }
        else {
            return false;
        }
    }

    template <typename Type>
    bool operator!=(delegate<Type>& other) const noexcept {
        return !(*this == other);
    }

    /**
     * @brief Clear bindings
     *
     */
    void reset() noexcept {
        context_  = nullptr;
        function_ = nullptr;
    }

    /**
     * @brief Get the function pointer
     *
     * @return function_type* Pointer of function
     */
    [[nodiscard]] function_type* target() const noexcept { return function_; }

    /**
     * @brief Get the context
     *
     * @return void const* Pointer to the context
     */
    [[nodiscard]] void const* context() const noexcept { return context_; }

private:
    void const* context_;
    function_type* function_;
};

} // namespace atom::utils
