#pragma once
#include <concepts>
#include <ranges>
#include <type_traits>
#include "core/pair.hpp"

// It is mainly used as a substitute when STL does not provide sufficient pipeline support, of
// course this refers to C++20.
// This pipeline system could work on cpp20 or higher.

namespace atom::utils {

/**
 * @brief Construct a range closure quickly by CRTP.
 *
 */
template <typename Derived>
requires(!std::ranges::range<Derived>)
struct pipeline_base {};

/**
 * @brief The custom result of pipeline operators.
 *
 * @tparam First The type of the first range closure.
 * @tparam Second The type of the second range closure.
 */
template <typename First, typename Second>
struct pipeline_result {
    template <typename Left, typename Right>
    constexpr pipeline_result(Left&& left, Right&& right) noexcept(
        std::is_nothrow_constructible_v<compressed_pair<First, Second>, Left, Right>)
        : closures_(std::forward<Left>(left), std::forward<Right>(right)) {}

    constexpr ~pipeline_result() noexcept(
        std::is_nothrow_destructible_v<compressed_pair<First, Second>>) = default;

    constexpr pipeline_result(const pipeline_result&) noexcept(
        std::is_nothrow_copy_constructible_v<compressed_pair<First, Second>>) = default;

    constexpr pipeline_result(pipeline_result&& that) noexcept(
        std::is_nothrow_move_constructible_v<compressed_pair<First, Second>>)
        : closures_(std::move(that.closures_)) {}

    constexpr pipeline_result& operator=(const pipeline_result&) noexcept(
        std::is_nothrow_copy_assignable_v<compressed_pair<First, Second>>) = default;

    constexpr pipeline_result& operator=(pipeline_result&& that) noexcept(
        std::is_nothrow_move_assignable_v<compressed_pair<First, Second>>) {
        closures_ = std::move(that.closures_);
    }

    template <std::ranges::range Rng>
    [[nodiscard]] constexpr auto operator()(Rng&& range) noexcept(noexcept(std::forward<Second>(
        closures_.second())(std::forward<First>(closures_.first())(std::forward<Rng>(range))))) {
        return std::forward<Second>(closures_.second())(
            std::forward<First>(closures_.first())(std::forward<Rng>(range)));
    }

    template <std::ranges::range Rng>
    [[nodiscard]] constexpr auto operator()(Rng&& range) const
        noexcept(noexcept(std::forward<Second>(closures_.second())(
            std::forward<First>(closures_.first())(std::forward<Rng>(range))))) {
        return std::forward<Second>(closures_.second())(
            std::forward<First>(closures_.first())(std::forward<Rng>(range)));
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
    std::remove_cvref_t<Closure>, atom::utils::pipeline_base<std::remove_cvref_t<Closure>>>
[[nodiscard]] constexpr inline auto operator|(Rng&& range, Closure&& closure) noexcept(
    noexcept(std::forward<Closure>(closure)(std::forward<Rng>(range)))) {
    return std::forward<Closure>(closure)(std::forward<Rng>(range));
}

/*! @cond TURN_OFF_DOXYGEN */
namespace atom::utils::internal {

template <typename Result>
struct is_pipeline_result : public std::false_type {};

template <typename First, typename Second>
struct is_pipeline_result<::atom::utils::pipeline_result<First, Second>> : public std::true_type {};

template <typename Result>
constexpr bool is_pipeline_result_t = is_pipeline_result<Result>::value;

} // namespace atom::utils::internal
/*! @endcond */

/**
 * @brief Construct a range
 * from a range and a pipeline result.
 */
template <std::ranges::range Rng, typename Result>
requires ::atom::utils::internal::is_pipeline_result_t<std::remove_cvref_t<Result>>
[[nodiscard]] constexpr inline auto operator|(Rng&& range, Result&& result) noexcept(
    noexcept(std::forward<Result>(result)(std::forward<Rng>(range)))) {
    return std::forward<Result>(result)(std::forward<Rng>(range));
}

// clang-format off
/**
 * @brief Construct a pipeline result
 * from two closures.
 */
template <typename Closure, typename Another>
requires std::derived_from<std::remove_cvref_t<Closure>,
            ::atom::utils::pipeline_base<std::remove_cvref_t<Closure>>> ||
         std::derived_from<std::remove_cvref_t<Another>,
            ::atom::utils::pipeline_base<std::remove_cvref_t<Another>>>
[[nodiscard]] constexpr inline auto operator|(
    Closure&& closure, Another&& another
) noexcept(std::is_nothrow_constructible_v<
            atom::utils::pipeline_result<Closure, Another>,
            Closure,
            Another>) {
    return UTILS pipeline_result<Closure, Another>(
        std::forward<Closure>(closure), std::forward<Another>(another)
    );
}
// clang-format on

// clang-format off
/**
 * @brief Construct a pipeline_result
 * from a pipeline_result and a closure.
 */
template <typename Result, typename Closure>
requires ::atom::utils::internal::is_pipeline_result_t<std::remove_cvref_t<Result>>
[[nodiscard]] constexpr inline auto operator|(
    Result&& closure, Closure&& another
) noexcept(std::is_nothrow_constructible_v<
            ::atom::utils::pipeline_result<Result, Closure>,
            Result,
            Closure>) {
    // clang-format on
    return UTILS pipeline_result<Result, Closure>(
        std::forward<Result>(closure), std::forward<Closure>(another));
}
