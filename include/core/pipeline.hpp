#pragma once
#include <type_traits>
#include "core/pair.hpp"

// It is mainly used as a substitute when STL does not provide sufficient pipeline support, of
// course this refers to C++20.
// This pipeline system could work on cpp20 or higher.

namespace atom::utils {

/**
 * @brief Construct a range closure quickly by alias.
 *
 * using pipeline_tag = pipeline_tag;
 */
struct pipeline_tag {
    pipeline_tag() = delete;
};

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

    /**
     * @brief Get out of the pipeline.
     *
     * Usually, the tparam will be a range, but not only.
     * It depends on the closure.
     * @tparam Arg This tparam could be deduced.
     */
    template <typename Arg>
    requires requires {
        std::forward<First>(std::declval<First>())(std::declval<Arg>());
        std::forward<Second>(std::declval<Second>())(
            std::forward<First>(std::declval<First>())(std::declval<Arg>()));
    }
    [[nodiscard]] constexpr auto operator()(Arg&& arg) noexcept(noexcept(std::forward<Second>(
        closures_.second())(std::forward<First>(closures_.first())(std::forward<Arg>(arg))))) {
        return std::forward<Second>(closures_.second())(
            std::forward<First>(closures_.first())(std::forward<Arg>(arg)));
    }

    /**
     * @brief Get out of the pipeline.
     *
     * Usually, the tparam will be a range, but not only.
     * It depends on the closure.
     * @tparam Arg This tparam could be deduced.
     */
    template <typename Arg>
    requires requires {
        std::forward<First>(std::declval<First>())(std::declval<Arg>());
        std::forward<Second>(std::declval<Second>())(
            std::forward<First>(std::declval<First>())(std::declval<Arg>()));
    }
    [[nodiscard]] constexpr auto operator()(Arg&& arg) const noexcept(noexcept(std::forward<Second>(
        closures_.second())(std::forward<First>(closures_.first())(std::forward<Arg>(arg))))) {
        return std::forward<Second>(closures_.second())(
            std::forward<First>(closures_.first())(std::forward<Arg>(arg)));
    }

private:
    compressed_pair<First, Second> closures_;
};

} // namespace atom::utils

namespace concepts {

template <typename Ty>
concept has_pipeline_tag = requires { typename Ty::pipeline_tag; };

} // namespace concepts

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
 * from a arg and a pipeline result.
 *
 * @tparam Arg Usually, this tparam will be a range, but not only.
 */
template <typename Arg, typename Result>
requires ::atom::utils::internal::is_pipeline_result_t<std::remove_cvref_t<Result>> && requires {
    std::forward<Result>(std::declval<Result>())(std::forward<Arg>(std::declval<Arg>()));
}
[[nodiscard]] constexpr inline auto operator|(Arg&& arg, Result&& result) noexcept(
    noexcept(std::forward<Result>(result)(std::forward<Arg>(arg)))) {
    return std::forward<Result>(result)(std::forward<Arg>(arg));
}

/**
 * @brief Construct a pipeline result
 * from two closures or a arg and a closure.
 */
template <typename Arg, typename Closure>
requires(concepts::has_pipeline_tag<std::remove_cvref_t<Arg>> ||
         concepts::has_pipeline_tag<std::remove_cvref_t<Closure>>) &&
        (requires {std::forward<Closure>(std::declval<Closure>())(std::forward<Arg>(std::declval<Arg>()));} || requires {
            ::atom::utils::pipeline_result<Arg, Closure>(std::declval<Arg>(), std::declval<Closure>());
        })
[[nodiscard]] constexpr inline auto operator|(Arg&& arg, Closure&& closure) noexcept(
    std::is_nothrow_constructible_v<
        atom::utils::pipeline_result<Arg, Closure>, Arg, Closure>) {
    if constexpr (requires {
                      std::forward<Closure>(std::declval<Closure>())(
                          std::forward<Arg>(std::declval<Arg>()));
                  }) {
        return std::forward<Closure>(closure)(std::forward<Arg>(arg));
    }
    else if constexpr (requires {
                           ::atom::utils::pipeline_result<Arg, Closure>(
                               std::declval<Arg>(), std::declval<Closure>());
                       }) {
        return ::atom::utils::pipeline_result<Arg, Closure>(
            std::forward<Arg>(arg), std::forward<Closure>(closure));
    }
    else {
        static_assert(false);
    }
}

/**
 * @brief Construct a pipeline_result
 * from a pipeline_result and a closure.
 */
template <typename Result, typename Closure>
requires ::atom::utils::internal::is_pipeline_result_t<std::remove_cvref_t<Result>> && requires {
    ::atom::utils::pipeline_result<Result, Closure>(
        std::declval<Result>(), std::declval<Closure>());
}
[[nodiscard]] constexpr inline auto operator|(Result&& result, Closure&& closure) noexcept(
    std::is_nothrow_constructible_v<
        ::atom::utils::pipeline_result<Result, Closure>, Result, Closure>) {
    return ::atom::utils::pipeline_result<Result, Closure>(
        std::forward<Result>(result), std::forward<Closure>(closure));
}
