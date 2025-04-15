#pragma once
#include <cstddef>
#include <ranges>
#include <type_traits>
#include "ranges.hpp"

namespace atom::utils {

namespace concepts {
template <typename Iter>
concept input_iterator = requires(const Iter iter) { *iter; };

template <typename Iter>
concept output_iterator = requires(Iter iter) { *iter; };

template <typename Iter>
concept forward_iterator = input_iterator<Iter> && requires(Iter iter) {
    ++iter;
    iter++;
};

template <typename Iter>
concept bidirectional_iterator = forward_iterator<Iter> && requires(Iter iter) {
    --iter;
    iter--;
};

template <typename Iter>
concept random_access_iterator =
    bidirectional_iterator<Iter> && requires(Iter iter) { iter += static_cast<unsigned>(-1); };
} // namespace concepts

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

}

// NOTE: iterator tags
// input_iterator_tag
// output_iterator_tag
// forward_iterator_tag
// bidirectional_iterator_tag
// random_access_iterator_tag
// contiguous_iterator_tag - new in cpp20

#if defined(__GNUC__)

template <typename Iter, typename Container>
constexpr bool operator==(
    const ::__gnu_cxx::__normal_iterator<Iter, Container>& lhs,
    const ::__gnu_cxx::__normal_iterator<const Iter, Container>& rhs) noexcept {
    return reinterpret_cast<::__gnu_cxx::__normal_iterator<const Iter, Container>&>(lhs) == rhs;
}

template <typename Ty, typename Container>
constexpr bool operator==(
    const ::__gnu_cxx::__normal_iterator<Ty, Container>& lhs,
    const ::__gnu_cxx::__normal_iterator<const Ty*, Container>& rhs) noexcept {
    return reinterpret_cast<::__gnu_cxx::__normal_iterator<const Ty*, Container>&>(lhs) == rhs;
}

template <typename Iter, typename Container>
constexpr bool operator!=(
    const ::__gnu_cxx::__normal_iterator<Iter, Container>& lhs,
    const ::__gnu_cxx::__normal_iterator<const Iter, Container>& rhs) noexcept {
    return reinterpret_cast<::__gnu_cxx::__normal_iterator<const Iter, Container>&>(lhs) != rhs;
}

template <typename Ty, typename Container>
constexpr bool operator!=(
    const ::__gnu_cxx::__normal_iterator<Ty, Container>& lhs,
    const ::__gnu_cxx::__normal_iterator<const Ty*, Container>& rhs) noexcept {
    return reinterpret_cast<::__gnu_cxx::__normal_iterator<const Ty*, Container>&>(lhs) != rhs;
}

#endif
