#pragma once
#include <utility>
#include "meta/sequence.hpp"

namespace atom::utils {

template <typename Ty>
struct _lambda_traits;

template <typename Class, typename Ret, typename... Args>
struct _lambda_traits<Ret (Class::*)(Args...) const> {
    using return_type                       = Ret;
    using function_type                     = Ret (*)(Args...);
    using args_type                         = std::tuple<Args...>;
    constexpr static inline size_t num_args = sizeof...(Args);
};

template <auto Lambda>
class lambda_wrapper {
public:
    using traits      = _lambda_traits<decltype(&decltype(Lambda)::operator())>;
    using return_type = typename traits::return_type;

    template <typename>
    struct invoker;

    template <std::size_t... Is>
    requires(std::index_sequence<Is...>().size() == traits::num_args)
    struct invoker<std::index_sequence<Is...>> {
        constexpr static inline return_type invoke(
            typename std::tuple_element_t<Is, typename traits::args_type>... args) {
            return Lambda(args...);
        }
    };
};

template <
    auto Lambda, typename LambdaTraits = _lambda_traits<decltype(&decltype(Lambda)::operator())>>
consteval inline auto addressof() noexcept -> typename LambdaTraits::function_type {
    return &lambda_wrapper<Lambda>::template invoker<integer_seq_t<LambdaTraits::num_args>>::invoke;
}

} // namespace atom::utils
