#pragma once
#include <concepts>
#include <tuple>
#include "core.hpp"
#include "core/pipline_result.hpp"

namespace atom::utils {

template <typename Fn, typename... Args>
class closure {
    using index_sequence = std::index_sequence_for<Args...>;
    using self_type      = closure;

public:
    static_assert((std::same_as<std::decay_t<Args>, Args> && ...));
    static_assert(std::is_empty_v<Fn> && std::is_default_constructible_v<Fn>);

    template <typename... ArgTypes>
    requires(std::same_as<std::decay_t<ArgTypes>, Args> && ...)
    explicit constexpr closure(ArgTypes&&... args
    ) noexcept(std::conjunction_v<std::is_nothrow_constructible<Args, ArgTypes>...>)
        : args_(std::make_tuple(std::forward<ArgTypes>(args)...)) {}

    template <typename Ty>
    requires std::invocable<Fn, Ty, Args&...>
    constexpr decltype(auto) operator()(Ty&& arg
    ) & noexcept(noexcept(call(*this, std::forward<Ty>(arg), index_sequence{}))) {
        return call(*this, std::forward<Ty>(arg), index_sequence{});
    }

    template <typename Ty>
    requires std::invocable<Fn, Ty, const Args&...>
    constexpr decltype(auto) operator()(Ty&& arg
    ) const& noexcept(noexcept(call(*this, std::forward<Ty>(arg), index_sequence{}))) {
        return call(*this, std::forward<Ty>(arg), index_sequence{});
    }

    template <typename Ty>
    requires std::invocable<Fn, Ty, Args...>
    constexpr decltype(auto) operator()(Ty&& arg
    ) && noexcept(noexcept(call(*this, std::forward<Ty>(arg), index_sequence{}))) {
        return call(*this, std::forward<Ty>(arg), index_sequence{});
    }

    template <typename Ty>
    requires std::invocable<Fn, Ty, const Args...>
    constexpr decltype(auto) operator()(Ty&& arg
    ) const&& noexcept(noexcept(call(*this, std::forward<Ty>(arg), index_sequence{}))) {
        return call(*this, std::forward<Ty>(arg), index_sequence{});
    }

    template <typename Closure>
    constexpr auto operator|(Closure closure) && {
        return pipeline_result<self_type, Closure>(std::move(*this), std::move(closure));
    }

private:
    template <typename SelfTy, typename Ty, size_t... Index>
    static constexpr decltype(auto)
        call(SelfTy&& self, Ty&& arg, std::index_sequence<Index...>) noexcept(noexcept(
            Fn{}(std::forward<Ty>(arg), std::get<Index>(std::forward<SelfTy>(self).args_)...)
        )) {
        static_assert(std::same_as<std::index_sequence<Index...>, index_sequence>);
        return Fn{}(std::forward<Ty>(arg), std::get<Index>(std::forward<SelfTy>(self).args_)...);
    }

    std::tuple<Args...> args_;
};

template <typename Fn, typename... Args>
constexpr static auto make_closure(Args&&... args) -> closure<Fn, std::decay_t<Args>...> {
    return closure<Fn, std::decay_t<Args>...>(std::forward<Args>(args)...);
}

} // namespace atom::utils
