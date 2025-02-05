#pragma once
#include <concepts>
#include <ranges>
#include <type_traits>
#include "core/pair.hpp"

namespace atom::utils {

// view: structure -> special range
// fn: view getter, wrapper operator()
// operator|: fn invoker, wrapper operator() as operator|

template <typename Derived>
struct pipeline_base {};

/**
 * @brief The custom result of pipeline operators.
 *
 * It is mainly used as a substitute when STL does not provide sufficient pipeline support, of
 * course this refers to C++20
 * @tparam Rng
 * @tparam Closure
 */
template <typename First, typename Second>
struct pipeline_result {
    template <typename Left, typename Right>
    constexpr pipeline_result(
        Left&& left, Right&& right
    ) noexcept(std::is_nothrow_constructible_v<compressed_pair<First, Second>, Left, Right>)
        : closures_(std::forward<Left>(left), std::forward<Right>(right)) {}

    constexpr ~pipeline_result(
    ) noexcept(std::is_nothrow_destructible_v<compressed_pair<First, Second>>) = default;

    constexpr pipeline_result(const pipeline_result&) noexcept(std::is_nothrow_copy_constructible_v<
                                                               compressed_pair<First, Second>>) =
        default;

    constexpr pipeline_result(pipeline_result&& that
    ) noexcept(std::is_nothrow_move_constructible_v<compressed_pair<First, Second>>)
        : closures_(std::move(that.closures_)) {}

    constexpr pipeline_result&
        operator=(const pipeline_result&) noexcept(std::is_nothrow_copy_assignable_v<
                                                   compressed_pair<First, Second>>) = default;

    constexpr pipeline_result& operator=(pipeline_result&& that
    ) noexcept(std::is_nothrow_move_assignable_v<compressed_pair<First, Second>>) {
        closures_ = std::move(that.closures_);
    }

    template <std::ranges::range Rng>
    constexpr auto operator()(Rng&& range) noexcept(
        noexcept(std::forward<Second>(closures_.second())(
            std::forward<First>(closures_.first())(std::forward<Rng>(range))
        ))
    ) {
        return std::forward<Second>(closures_.second())(
            std::forward<First>(closures_.first())(std::forward<Rng>(range))
        );
    }

    template <std::ranges::range Rng>
    constexpr auto operator()(Rng&& range) const
        noexcept(noexcept(std::forward<Second>(closures_.second())(
            std::forward<First>(closures_.first())(std::forward<Rng>(range))
        ))) {
        return std::forward<Second>(closures_.second())(
            std::forward<First>(closures_.first())(std::forward<Rng>(range))
        );
    }

private:
    compressed_pair<First, Second> closures_;
};

} // namespace atom::utils

/**
 * @brief Consturct a range
 * from a range and a closure.
 */
template <std::ranges::range Rng, typename Closure>
requires std::derived_from<
    std::remove_cvref_t<Closure>,
    atom::utils::pipeline_base<std::remove_cvref_t<Closure>>>
constexpr auto operator|(Rng&& range, Closure&& closure) noexcept(
    noexcept(std::forward<Closure>(closure)(std::forward<Rng>(range)))
) {
    return std::forward<Closure>(closure)(std::forward<Rng>(range));
}

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

template <typename Result>
struct is_pipeline_result : public std::false_type {};

template <typename First, typename Second>
struct is_pipeline_result<::atom::utils::pipeline_result<First, Second>> : public std::true_type {};

template <typename Result>
constexpr bool is_pipeline_result_t = is_pipeline_result<Result>::value;

} // namespace internal
/*! @endcond */

/**
 * @brief Construct a range
 * from a range and a pipeline result.
 */
template <std::ranges::range Rng, typename Result>
requires internal::is_pipeline_result_t<std::remove_cvref_t<Result>>
constexpr auto operator|(Rng&& range, Result&& result) noexcept(
    noexcept(std::forward<Result>(result)(std::forward<Rng>(range)))
) {
    return std::forward<Result>(result)(std::forward<Rng>(range));
}

/**
 * @brief Construct a pipeline result
 * from two closure.
 */
template <typename Closure, typename Another>
requires std::derived_from<
             std::remove_cvref_t<Closure>,
             ::atom::utils::pipeline_base<std::remove_cvref_t<Closure>>> ||
         std::derived_from<
             std::remove_cvref_t<Another>,
             ::atom::utils::pipeline_base<std::remove_cvref_t<Another>>>
constexpr auto operator|(
    Closure&& closure, Another&& another
) noexcept(std::
               is_nothrow_constructible_v<
                   atom::utils::pipeline_result<Closure, Another>,
                   Closure,
                   Another>) {
    return UTILS pipeline_result<Closure, Another>(
        std::forward<Closure>(closure), std::forward<Another>(another)
    );
}
