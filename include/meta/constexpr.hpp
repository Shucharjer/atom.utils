#pragma once
#include <type_traits>

#if __has_include(<meta.hpp>)
    #include "meta.hpp"
#endif

#if __has_include(<meta/list.hpp>)
    #include "meta/list.hpp"
#else
namespace atom::utils {
template <typename...>
struct type_list {};
} // namespace atom::utils
#endif

namespace atom::utils {

template <auto expr, typename Expr = decltype(expr)>
struct expression_traits;

template <auto expr, typename Ret, typename... Args>
struct expression_traits<expr, Ret (*)(Args...)> {
    constexpr static std::size_t args_count = sizeof...(Args);
    using args_type                         = type_list<Args...>;
    constexpr static bool is_constexpr =
        requires { typename std::bool_constant<(expr(Args{}...), false)>; };
};

template <auto expr, typename Ret, typename Clazz, typename... Args>
struct expression_traits<expr, Ret (Clazz::*)(Args...)> {
    constexpr static std::size_t args_count = sizeof...(Args);
    using args_type                         = type_list<Args...>;
    constexpr static bool is_constexpr =
        requires { typename std::bool_constant<(Clazz{}.*expr(Args()...), false)>; };
};

template <auto expr, typename Ret, typename Clazz, typename... Args>
struct expression_traits<expr, Ret (Clazz::*)(Args...) const> {
    constexpr static std::size_t args_count = sizeof...(Args);
    using args_type                         = type_list<Args...>;
    constexpr static bool is_constexpr =
        requires { typename std::bool_constant<(Clazz{}.*expr(Args{}...), false)>; };
};

template <auto expr, typename Expr>
struct expression_traits {
    static_assert(std::is_class_v<Expr>);

    template <typename>
    struct helper;

    template <typename Ret, typename Clazz, typename... Args>
    struct helper<Ret (Clazz::*)(Args...) const> {
        constexpr static Ret invoke() { return expr(Args{}...); }

        constexpr static std::size_t args_count = sizeof...(Args);
        using args_type                         = type_list<Args...>;
        constexpr static bool is_constexpr =
            requires { typename std::bool_constant<(invoke(), false)>; };
    };

    using helper_t = helper<decltype(&Expr::operator())>;

    constexpr static std::size_t args_count = helper_t::args_count;
    using args_type                         = helper_t::args_type;
    constexpr static bool is_constexpr      = helper_t::is_constexpr;
};

template <auto expr>
consteval bool is_constexpr() {
    return expression_traits<expr>::is_constexpr;
}

} // namespace atom::utils
