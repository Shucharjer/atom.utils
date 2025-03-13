#pragma once
#include <cstddef>
#include <ranges>
#include <type_traits>
#include "ranges.hpp"

namespace atom::utils {

namespace ranges {
/**
 * @brief Phony input iterator, for lazy construction with ranges.
 *
 * Can not really iterate.
 * @tparam Rng
 */
template <std::ranges::input_range Rng>
struct phony_input_iterator {
    using iterator_category = std::input_iterator_tag;
    using value_type        = std::ranges::range_value_t<Rng>;
    using difference_type   = ptrdiff_t;
    using pointer           = std::add_pointer_t<std::ranges::range_reference_t<Rng>>;
    using reference         = std::ranges::range_reference_t<Rng>;

    reference operator*() const = delete;
    pointer operator->() const  = delete;

    phony_input_iterator& operator++()   = delete;
    phony_input_iterator operator++(int) = delete;

    bool operator==(const phony_input_iterator&) const = delete;
    bool operator!=(const phony_input_iterator&) const = delete;
};

} // namespace ranges

// NOTE: iterator tags
// input_iterator_tag
// output_iterator_tag
// forward_iterator_tag
// bidirectional_iterator_tag
// random_access_iterator_tag
// contiguous_iterator_tag - new in cpp20

template <typename Derived>
class iterator {
public:
    using value_type      = typename Derived::value_type;
    using reference       = typename Derived::reference;
    using const_reference = typename Derived::const_reference;
    using pointer         = typename Derived::pointer;
    using const_pointer   = typename Derived::const_pointer;

    using difference_type   = typename Derived::difference_type;
    using iterator_category = typename Derived::iterator_category;

    iterator() noexcept                           = default;
    iterator(const iterator&) noexcept            = default;
    iterator(iterator&&) noexcept                 = default;
    iterator& operator=(const iterator&) noexcept = default;
    iterator& operator=(iterator&&) noexcept      = default;
    ~iterator() noexcept                          = default;

    constexpr auto& operator++() noexcept(noexcept(static_cast<Derived*>(this)->operator++()))
    requires requires(Derived iter) { ++iter; }
    {
        static_cast<Derived*>(this)->operator++();
    }
    constexpr auto operator++(int) noexcept(noexcept(static_cast<Derived*>(this)->operator++(int{
        1 })))
    requires requires(Derived iter) { iter++; }
    {
        static_cast<Derived*>(this)->operator++(int{ 1 });
    }

    constexpr auto& operator--() noexcept(noexcept(static_cast<Derived*>(this)->operator--()))
    requires requires(Derived iter) { --iter; }
    {
        static_cast<Derived*>(this)->operator--();
    }
    constexpr auto operator--(int) noexcept(noexcept(static_cast<Derived*>(this)->operator--(int{
        1 })))
    requires requires(Derived iter) { iter--; }
    {
        static_cast<Derived*>(this)->operator--(int{ 1 });
    }

    constexpr auto& operator+=(const difference_type diff) noexcept(
        noexcept(static_cast<Derived*>(this)->operator+=(diff)))
    requires requires(Derived iter) { iter += std::declval<difference_type>(); }
    {
        static_cast<Derived*>(this)->operator+=(diff);
    }
    constexpr auto operator+(const difference_type diff) noexcept(
        noexcept(static_cast<Derived*>(this)->operator+(diff)))
    requires requires(Derived iter) { iter + std::declval<difference_type>(); }
    {
        return static_cast<Derived*>(this)->operator+(diff);
    }

    constexpr auto& operator-=(const difference_type diff) noexcept(
        noexcept(static_cast<Derived*>(this)->operator-(diff)))
    requires requires(Derived iter) { iter -= std::declval<difference_type>(); }
    {
        static_cast<Derived*>(this)->operator-=(diff);
    }
    constexpr auto operator-(const difference_type diff) noexcept(
        noexcept(static_cast<Derived*>(this)->operator-(diff)))
    requires requires(Derived iter) { iter - std::declval<difference_type>(); }
    {
        return static_cast<Derived*>(this)->operator-(diff);
    }

    constexpr reference operator*() noexcept(noexcept(static_cast<Derived*>(this)->operator*()))
    requires requires(Derived iter) { *iter; }
    {
        return static_cast<Derived*>(this)->operator*();
    }

    constexpr const_reference operator*() const
        noexcept(noexcept(static_cast<Derived*>(this)->operator*()))
    requires requires(Derived iter) { *iter; }
    {
        return static_cast<Derived*>(this)->operator*();
    }

    constexpr auto& operator[](const difference_type diff) noexcept(
        noexcept(static_cast<Derived*>(this)->operator[](diff)))
    requires requires(Derived iter) { iter[std::declval<difference_type>()]; }
    {
        return static_cast<Derived*>(this)->operator[](diff);
    }

    constexpr auto& operator[](const difference_type diff) const
        noexcept(noexcept(static_cast<Derived*>(this)->operator[](diff)))
    requires requires(Derived iter) { iter[std::declval<difference_type>()]; }
    {
        return static_cast<Derived*>(this)->operator[](diff);
    }
};

} // namespace atom::utils

#ifdef __GNUC__

template <typename Iter, typename Container>
constexpr bool operator==(
    const ::__gnu_cxx::__normal_iterator<Iter, Container>& lhs,
    const ::__gnu_cxx::__normal_iterator<const Iter, Container>& rhs) noexcept {
    return reinterpret_cast<::__gnu_cxx::__normal_iterator<const Iter, Container>&>(lhs) == rhs;
}

template <typename Iter, typename Container>
constexpr bool operator!=(
    const ::__gnu_cxx::__normal_iterator<Iter, Container>& lhs,
    const ::__gnu_cxx::__normal_iterator<const Iter, Container>& rhs) noexcept {
    return reinterpret_cast<::__gnu_cxx::__normal_iterator<const Iter, Container>&>(lhs) != rhs;
}

#endif
