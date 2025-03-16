#pragma once
#include <concepts>
#include <cstddef>
#include <ranges>
#include <tuple>
#include <type_traits>
#include "core/pipeline.hpp"
#include "core/type_traits.hpp"

namespace atom::utils {

template <std::ranges::range Rng, size_t Index, bool IsConst>
struct element_iterator {
    using value_type      = std::tuple_element_t<Index, std::ranges::range_value_t<Rng>>;
    using difference_type = ptrdiff_t;

    constexpr element_iterator() = default;

    constexpr element_iterator(std::ranges::iterator_t<Rng> iter) noexcept(
        std::is_nothrow_move_constructible_v<std::ranges::iterator_t<Rng>>)
        : iter_(std::move(iter)) {}

    [[nodiscard]] constexpr decltype(auto) operator*() {
        if constexpr (requires { std::get<Index>(*iter_); }) {
            return ::std::get<Index>(*iter_);
        }
        else if constexpr (requires { iter_->template get<Index>(); }) {
            return iter_->template get<Index>();
        }
        else if constexpr (requires { get<Index>(*iter_); }) {
            return get<Index>(*iter_);
        }
        else {
            static_assert(false, "No suitable method to get the value.");
        }
    }

    [[nodiscard]] constexpr decltype(auto) operator->() noexcept { return iter_; }

    [[nodiscard]] constexpr decltype(auto) operator->() const noexcept { return iter_; }

    constexpr element_iterator& operator++() noexcept(noexcept(++iter_)) {
        ++iter_;
        return *this;
    }

    constexpr element_iterator operator++(int) noexcept(
        noexcept(++iter_) && std::is_nothrow_copy_constructible_v<std::ranges::iterator_t<Rng>>) {
        auto temp = *this;
        ++iter_;
        return temp;
    }

    constexpr element_iterator& operator--() noexcept(noexcept(--iter_)) {
        --iter_;
        return *this;
    }

    constexpr element_iterator operator--(int) noexcept(
        noexcept(--iter_) && std::is_nothrow_copy_constructible_v<std::ranges::iterator_t<Rng>>) {
        auto temp = *this;
        --iter_;
        return temp;
    }

    constexpr element_iterator& operator+=(const difference_type offset) noexcept(
        noexcept(iter_ += offset))
    requires std::ranges::random_access_range<Rng>
    {
        iter_ += offset;
        return *this;
    }

    constexpr element_iterator operator+(const difference_type offset)
    requires std::ranges::random_access_range<Rng>
    {
        auto temp = *this;
        temp += offset;
        return temp;
    }

    constexpr element_iterator& operator-=(const difference_type offset) noexcept(
        noexcept(iter_ -= offset))
    requires std::ranges::random_access_range<Rng>
    {
        iter_ -= offset;
        return *this;
    }

    constexpr element_iterator operator-(const difference_type offset)
    requires std::ranges::random_access_range<Rng>
    {
        auto temp = *this;
        temp -= offset;
        return temp;
    }

private:
    std::ranges::iterator_t<Rng> iter_;
};

/**
 * @brief Elements view, but supports user defined get.
 *
 * @tparam Vw
 * @tparam Index
 */
template <std::ranges::input_range Vw, size_t Index>
class elements_view {
public:
    /**
     * @brief Constuct a elements view.
     *
     * @warning Move then constructing.
     */
    constexpr explicit elements_view(Vw range) noexcept(std::is_nothrow_constructible_v<Vw>)
        : range_(std::move(range)) {}

    constexpr auto begin() { return element_iterator<Vw, Index, false>(range_.begin()); }

    constexpr auto begin() const {
        if constexpr (requires { range_.cbegin(); }) {
            return element_iterator<Vw, Index, true>(range_.cbegin());
        }
        else if constexpr (requires { range_.begin(); }) {
            return element_iterator<Vw, Index, true>(range_.begin());
        }
        else {
            static_assert(false, "No suitable method to get a const iterator");
        }
    }

    constexpr auto end() { return element_iterator<Vw, Index, false>(range_.cbegin()); }

    constexpr auto end() const {
        if constexpr (requires { range_.cend(); }) {
            return element_iterator<Vw, Index, true>(range_.cend());
        }
        else if constexpr (requires { range_.end(); }) {
            return element_iterator<Vw, Index, true>(range_.end());
        }
        else {
            static_assert(false, "No suitable method to get a const iterator");
        }
    }

    constexpr auto size() const noexcept
    requires std::ranges::sized_range<const Vw>
    {
        return std::ranges::size(range_);
    }

private:
    Vw range_;
};

template <size_t Index>
struct element_fn {
    using pipeline_tag = pipeline_tag;

    template <std::ranges::viewable_range Rng>
    [[nodiscard]] constexpr auto operator()(Rng&& range) const {
        return elements_view<std::views::all_t<Rng>, Index>{ std::forward<Rng>(range) };
    }
};

template <size_t Index>
constexpr element_fn<Index> elements;

constexpr inline auto keys   = elements<0>;
constexpr inline auto values = elements<1>;

struct concat_fn {
    using pipeline_tag = pipeline_tag;

    template <std::ranges::viewable_range Rng>
    constexpr auto operator()(Rng&& range) const noexcept {
        return make_closure<concat_fn>(std::views::all(std::forward<Rng>(range)));
    }

    template <std::ranges::viewable_range Lhs, std::ranges::viewable_range Rhs>
    constexpr auto operator()(Lhs&& lhs, Rhs&& rhs) {
        using lvalue_t = std::ranges::range_value_t<Lhs>;
        using rvalue_t = std::ranges::range_value_t<Rhs>;
        if constexpr (
            is_pair_v<lvalue_t> || is_std_tuple_v<lvalue_t> || is_pair_v<rvalue_t> ||
            is_std_tuple_v<rvalue_t>) {
            // combine as tuple
        }
        else {
            // combine as pair
        }
    }
};

constexpr inline concat_fn concat;

} // namespace atom::utils
