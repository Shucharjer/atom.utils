#pragma once
#include <cstddef>
#include <ranges>
#include <type_traits>
#include "core/pipeline.hpp"
#include "iterator.hpp"

namespace atom::utils {

template <std::ranges::range Rng, size_t Index, bool IsConst>
struct element_iterator : public iterator<element_iterator<Rng, Index, IsConst>, IsConst> {
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
    {}

private:
    std::ranges::iterator_t<Rng> iter_;
};

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

    constexpr auto begin() {}

    constexpr auto begin() const {}

    constexpr auto end() {}

    constexpr auto end() const {}

    constexpr auto size() noexcept
    requires std::ranges::sized_range<const Vw>
    {
        return std::ranges::size(range_);
    }

private:
    Vw range_;
};

template <size_t Index>
struct element_fn : public pipeline_base<element_fn<Index>> {
    template <std::ranges::viewable_range Rng>
    [[nodiscard]] constexpr auto operator()(Rng&& range) const {
        return elements_view<std::views::all_t<Rng>, Index>{ std::forward<Rng>(range) };
    }
};

template <size_t Index>
constexpr element_fn<Index> elements;

constexpr inline auto keys   = elements<0>;
constexpr inline auto values = elements<1>;

} // namespace atom::utils
