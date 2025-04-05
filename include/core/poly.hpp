#pragma once
#include <any>
#include <tuple>
#include <type_traits>
#include <utility>

#if __has_include(<core/langdef.hpp>)
    #include "core/langdef.hpp"
#else
    #ifdef _MSC_VER
        #define ATOM_FORCE_INLINE __forceinline
    #elif defined(__GNUC__) || defined(__clang__)
        #define ATOM_FORCE_INLINE inline __attribute__((__always_inline__))
    #else
        #define ATOM_FORCE_INLINE inline
    #endif

    #if defined(_DEBUG)
        #define ATOM_RELEASE_INLINE
    #else
        #define ATOM_RELEASE_INLINE ATOM_FORCE_INLINE
    #endif
#endif

#if __has_include(<core/type_traits.hpp>)
    #include "core/type_traits.hpp"
#else

namespace atom::utils {

template <typename To, typename From>
struct same_cv {
    using type = To;
};

template <typename To, typename From>
struct same_cv<To, const From> {
    using type = const To;
};

template <typename To, typename From>
struct same_cv<To, volatile From> {
    using type = volatile To;
};

template <typename To, typename From>
struct same_cv<To, const volatile From> {
    using type = const volatile To;
};

template <typename To, typename From>
using same_cv_t = typename same_cv<To, From>::type;

} // namespace atom::utils

#endif

namespace atom::utils {

template <typename...>
struct invoke_list {};

template <auto...>
struct value_list {};

template <std::size_t Index, typename ValueList>
struct value_list_element;

template <auto Arg, auto... Args>
struct value_list_element<0U, value_list<Arg, Args...>> {
    constexpr static auto value = Arg;
};

template <std::size_t Index, auto Arg, auto... Args>
struct value_list_element<Index, value_list<Arg, Args...>> {
    constexpr static auto value = value_list_element<Index - 1, value_list<Args...>>::value;
};

template <std::size_t Index, typename ValueList>
constexpr inline auto value_list_element_t = value_list_element<Index, ValueList>::value;

/**
 * @brief Inspector class used to deduce the function type.
 *
 * @note Generally, you have no reason to use this.
 */
struct poly_inspector {
    template <typename Ty>
    operator Ty&&() const;

    template <std::size_t Index, typename... Args>
    [[nodiscard]] poly_inspector invoke(Args&&... args) const;

    template <std::size_t Index, typename... Args>
    [[nodiscard]] poly_inspector invoke(Args&&... args);
};

template <typename... Args>
consteval static inline auto invoke_list_size(const invoke_list<Args...>* const) noexcept {
    return sizeof...(Args);
}

/**
 * @brief Table contains polymorphic functions.
 *
 * @tparam Object The objects have same polymorphism.
 */
template <typename Object>
class vtable {
    using inspector = typename Object::template interface<poly_inspector>;

    template <typename Ret, typename Clazz, typename... Args>
    requires std::is_base_of_v<std::remove_cvref_t<Clazz>, inspector>
    constexpr static auto entry(Ret (*)(Clazz&, Args...)) noexcept
        -> Ret (*)(same_cv_t<std::any, Clazz>&, Args...) {
        return nullptr;
    }

    template <typename Ret, typename Clazz, typename... Args>
    constexpr static auto entry(Ret (*)(Args...)) noexcept
        -> Ret (*)(same_cv_t<std::any, Clazz>&, Args...) {
        return nullptr;
    }

    template <typename Ret, typename Clazz, typename... Args>
    constexpr static auto entry(Ret (Clazz::*)(Args...)) noexcept -> Ret (*)(std::any&, Args...) {
        return nullptr;
    }

    template <typename Ret, typename Clazz, typename... Args>
    constexpr static auto entry(Ret (Clazz::*)(Args...) const) noexcept
        -> Ret (*)(const std::any&, Args...) {
        return nullptr;
    }

    template <typename... Func>
    [[nodiscard]] constexpr static auto generate_vtable(invoke_list<Func...>) noexcept {
        static_assert((std::is_function_v<Func> && ...));
        return decltype(std::make_tuple(entry(std::declval<Func inspector::*>())...)){};
    }

    using value_type = decltype(generate_vtable(Object{}));

    template <auto... Candidate>
    constexpr static value_type generate_vtable(value_list<Candidate...>) noexcept {
        return std::make_tuple(entry(Candidate)...);
    }

    template <typename Ty, auto Candidate, typename Ret, typename Any, typename... Args>
    constexpr static void initialize_vtable(Ret (*&entry)(Any&, Args...)) noexcept {
        if constexpr (std::is_invocable_r_v<Ret, decltype(Candidate), Args...>) {
            entry = [](Any&, Args... args) -> Ret {
                return std::invoke(Candidate, std::forward<Args>(args)...);
            };
        }
        else {
            entry = [](Any& instance, Args... args) -> Ret {
                return static_cast<Ret>(std::invoke(
                    Candidate, std::any_cast<same_cv_t<Ty, Any>&>(instance),
                    std::forward<Args>(args)...));
            };
        }
    }

    template <typename Ty, std::size_t... Is>
    consteval static value_type spawn_vtable(std::index_sequence<Is...>) noexcept {
        auto table = generate_vtable(typename Object::template implementation<Ty>{});
        ((initialize_vtable<
             Ty, value_list_element_t<Is, typename Object::template implementation<Ty>>>(
             std::get<Is>(table))),
         ...);
        return table;
    }

    template <typename Ty>
    constexpr static value_type table_ = spawn_vtable<Ty>(
        std::make_index_sequence<invoke_list_size(static_cast<Object*>(nullptr))>());

public:
    using type = value_type;

    template <typename Ty>
    consteval static auto instance() -> const value_type {
        return table_<Ty>;
    }
};

template <typename Poly>
struct poly_base {
    template <std::size_t Index, typename... Args>
    [[nodiscard]] constexpr decltype(auto) invoke(const poly_base& self, Args&&... args) const {
        const auto& poly = static_cast<const Poly&>(self);

        if constexpr (std::is_function_v<std::remove_pointer_t<decltype(poly.vtable_)>>) {
            return poly.vtable_(poly.any_, std::forward<Args>(args)...);
        }
        else {
            return std::get<Index>(poly.vtable_)(poly.any_, std::forward<Args>(args)...);
        }
    }

    template <std::size_t Index, typename... Args>
    constexpr decltype(auto) invoke(poly_base& self, Args&&... args) {
        auto& poly = static_cast<Poly&>(self);

        if constexpr (std::is_function_v<std::remove_pointer_t<decltype(poly.vtable_)>>) {
            return poly.vtable_(poly.any_, std::forward<Args>(args)...);
        }
        else {
            return std::get<Index>(poly.vtable_)(poly.any_, std::forward<Args>(args)...);
        }
    }
};

/**
 * @brief Polymorphism without runtime invoke overhead.
 *
 * @tparam Object Object that has a member struct template named interface.
 */
template <typename Object>
class poly : private Object::template interface<poly_base<poly<Object>>> {
    friend class poly_base<poly>;

public:
    using interface   = Object::template interface<poly_base<poly<Object>>>;
    using vtable_type = typename vtable<Object>::type;

    poly() noexcept = default;

    template <typename Ty>
    requires(!std::is_same_v<poly, std::remove_cvref_t<Ty>>)
    constexpr poly(Ty&& val)
        : any_(std::forward<Ty>(val)),
          vtable_(vtable<Object>::template instance<std::remove_cvref_t<Ty>>()) {}

    template <typename Ty, typename... Args>
    constexpr poly(Args&&... args)
        : any_(std::make_any<Ty>(std::forward<Args>(args)...)),
          vtable_(vtable<Object>::template instance<std::remove_cvref_t<Ty>>()) {}

    constexpr poly(const poly& that) : any_(that.any_), vtable_() {}

    constexpr poly(poly&& that) noexcept : any_(std::move(that.any_)), vtable_() {}

    constexpr poly& operator=(const poly& that) {
        if (this != &that) [[likely]] {
            any_    = that.any_;
            vtable_ = that.vtable_;
        }
        return *this;
    }

    constexpr poly& operator=(poly&& that) noexcept {
        if (this != &that) [[likely]] {
            any_    = std::move(that.any_);
            vtable_ = std::move(that.vtable_);
        }
        return *this;
    }

    ~poly() = default;

    interface* operator->() noexcept { return this; }
    interface* operator->() const noexcept { return this; }

    template <typename Ty = void>
    Ty* data() noexcept {
        return std::any_cast<Ty>(any_);
    }

    template <typename Ty = void>
    const Ty* data() const noexcept {
        return std::any_cast<Ty>(any_);
    }

    constexpr operator bool() const noexcept { return any_.has_value(); }

private:
    std::any any_;
    vtable_type vtable_;
};

/**
 * @brief Call functions with polymorphism.
 *
 * @tparam Index The index of the function.
 * @tparam Poly interface<Base>, further derived from Base, which is poly_base<poly<Object>>.
 * @tparam Args Argument types for invocation.
 * @param self An instance of poly_base<poly<Object>>, usually pass by *this
 * @param args Arguments for invocation.
 * @return decltype(auto) The return value of the function.
 */
template <std::size_t Index, typename T, typename... Args>
ATOM_FORCE_INLINE constexpr decltype(auto) poly_call(T&& self, Args&&... args) {
    return std::forward<T>(self).template invoke<Index>(self, std::forward<Args>(args)...);
}

template <typename... Args>
invoke_list<Args...> as_invoke_list(const invoke_list<Args...>&);

template <typename Lhs, typename Rhs>
struct concat_invoke_list;

template <typename... Llist, typename... Rlist>
struct concat_invoke_list<invoke_list<Llist...>, invoke_list<Rlist...>> final {
    using type = invoke_list<Llist..., Rlist...>;
};

template <typename Lhs, typename Rhs>
using concat_invoke_list_t = typename concat_invoke_list<Lhs, Rhs>::type;

template <typename InvokeList, typename... Args>
struct append_invoke_list;

template <typename... Tys, typename... Append>
struct append_invoke_list<invoke_list<Tys...>, Append...> final {
    using type = invoke_list<Tys..., Append...>;
};

template <typename InvokeList, typename... Args>
using append_invoke_list_t = typename append_invoke_list<InvokeList, Args...>::type;

template <typename Lhs, typename Rhs>
struct concat_value_list;

template <auto... LValues, auto... RValues>
struct concat_value_list<value_list<LValues...>, value_list<RValues...>> final {
    using type = value_list<LValues..., RValues...>;
};

template <typename Lhs, typename Rhs>
using concat_value_list_t = typename concat_value_list<Lhs, Rhs>::type;

template <typename ValueList, auto... Append>
struct append_value_list;

template <auto... Args, auto... Append>
struct append_value_list<value_list<Args...>, Append...> final {
    using type = value_list<Args..., Append...>;
};

template <typename ValueList, auto... Append>
using append_value_list_t = typename append_value_list<ValueList, Append...>::type;

} // namespace atom::utils
