#pragma once
#include <ranges>
#include <type_traits>
#include "core.hpp"

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
 * @brief The custom result of pipeline operator between two closures. It's a special closure.
 *
 * @tparam First The type of the first range closure.
 * @tparam Second The type of the second range closure.
 */
template <typename First, typename Second>
struct pipeline_result {
    using pipeline_tag = utils::pipeline_tag;

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

namespace concepts {

/**
 * @brief A type has type alias named pipeline_tag
 */
template <typename Ty>
concept has_pipeline_tag = requires { typename Ty::pipeline_tag; };

} // namespace concepts

} // namespace atom::utils

/**
 * @brief Construct an object by pipeline operator.
 *
 * @tparam Arg This tparam could be deduced automatically.
 * @tparam Closure This tparam could be deduced automatically.
 * @param[in] arg A closure object or arg could be pass to the closure.
 * @param[in] closure A closure object.
 */
template <typename Arg, typename Closure>
requires(atom::utils::concepts::has_pipeline_tag<std::remove_cvref_t<Closure>>)
[[nodiscard]] constexpr inline auto operator|(Arg&& arg, Closure&& closure) noexcept(
    ((std::ranges::range<Arg> ||
      requires { std::forward<Closure>(closure)(std::forward<Arg>(arg)); }) &&
     noexcept((std::forward<Closure>(closure))(std::forward<Arg>(arg)))) ||
    noexcept(atom::utils::pipeline_result<Arg, Closure>(
        std::forward<Arg>(arg), std::forward<Closure>(closure)))) {
    if constexpr (std::ranges::range<Arg>) {
        return (std::forward<Closure>(closure))(std::forward<Arg>(arg));
    }
    else if constexpr (requires { closure(arg); }) {
        return (std::forward<Closure>(closure))(std::forward<Arg>(arg));
    }
    else {
        return ::atom::utils::pipeline_result<Arg, Closure>(
            std::forward<Arg>(arg), std::forward<Closure>(closure));
    }
}

/**
 * @brief Construct a pipeline result.
 *
 * @tparam Closure A closure with pipeline_tag. Could be deduced.
 * @tparam WildClosure A type maybe has pipeline_tag. Could be deduced.
 * @param[in] closure
 * @param[in] wild
 * @warning Whether the tparam WildClosure is a closure, this function would return a
 * pipeline_result object.
 */
template <typename Closure, typename WildClosure>
requires atom::utils::concepts::has_pipeline_tag<std::remove_cvref_t<Closure>>
[[nodiscard]] constexpr inline auto operator|(Closure&& closure, WildClosure&& wild) noexcept(
    noexcept(atom::utils::pipeline_result<Closure, WildClosure>(
        std::forward<Closure>(closure), std::forward<WildClosure>(wild)))) {
    return atom::utils::pipeline_result<Closure, WildClosure>(
        std::forward<Closure>(closure), std::forward<WildClosure>(wild));
}
