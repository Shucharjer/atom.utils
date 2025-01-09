#pragma once
#include <concepts>
#include <iterator>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include "concepts.h"
#include "iterator.h"
#include "utils_macro.h"

#define URANGES ::atom::utils::ranges::

namespace atom::utils::ranges {

template <typename Rng, typename Container>
concept ref_convertible =
    !std::ranges::input_range<Container> ||
    std::convertible_to<std::ranges::range_reference_t<Rng>, std::ranges::range_value_t<Container>>;

template <typename Rng, typename Container, typename... Args>
concept common_constructible =
    ::atom::utils::concepts::common_range<Rng> &&
    // using of iterator_category
    requires { typename std::iterator_traits<std::ranges::iterator_t<Rng>>::iterator_category; } &&
    // input_range_tag or derived from input_range_tag
    (std::same_as<
         typename std::iterator_traits<std::ranges::iterator_t<Rng>>::iterator_category,
         std::input_iterator_tag> ||
     std::derived_from<
         typename std::iterator_traits<std::ranges::iterator_t<Rng>>::iterator_category,
         std::input_iterator_tag>) &&
    // constructible from iterator (and arguments)
    std::constructible_from<
        Container,
        std::ranges::iterator_t<Rng>,
        std::ranges::iterator_t<Rng>,
        Args...>;

template <typename Container, typename Reference>
// clang-format off
concept can_emplace_back = requires(Container& cnt) {
    // clang-format on
    cnt.emplace_back(std::declval<Reference>());
};

template <typename Container, typename Reference>
concept can_push_back = requires(Container& cnt) { cnt.push_back(std::declval<Reference>()); };

template <typename Container, typename Reference>
// clang-format off
concept can_emplace_end = requires(Container& cnt) {
    // clang-format on
    cnt.emplace(cnt.end(), std::declval<Reference>());
};

template <typename Container, typename Reference>
// clang-format off
concept can_insert_end = requires(Container& cnt) {
    // clang-format on
    cnt.insert(cnt.end(), std::declval<Reference>());
};

template <typename Rng, typename Container, typename... Args>
concept constructible_appendable =
    std::constructible_from<Container, Args...> &&
    (can_emplace_back<Container, std::ranges::range_reference_t<Rng>> ||
     can_push_back<Container, std::ranges::range_reference_t<Rng>> ||
     can_emplace_end<Container, std::ranges::range_reference_t<Rng>> ||
     can_insert_end<Container, std::ranges::range_reference_t<Rng>>);

template <typename Rng, typename Closure>
struct pipeline_result {
    template <typename NextClosure>
    constexpr auto operator|(NextClosure closure) && {
        return pipeline_result<pipeline_result<Rng, Closure>, NextClosure>{ *this, closure };
    }
};

template <typename Rng, typename Closure>
constexpr auto operator|(Rng&& range, Closure&& closure) {
    return pipeline_result<std::decay_t<Rng>, std::decay_t<Closure>>(
        std::forward<Rng>(range), std::forward<Closure>(closure)
    );
}

template <typename Fn, typename... Args>
class range_closure {
    using index_sequence = std::index_sequence_for<Args...>;

public:
    static_assert((std::same_as<std::decay_t<Args>, Args> && ...));
    static_assert(std::is_empty_v<Fn> && std::is_default_constructible_v<Fn>);

    template <typename... ArgTypes>
    requires(std::same_as<std::decay_t<ArgTypes>, Args> && ...)
    explicit constexpr range_closure(ArgTypes&&... args
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
        return pipeline_result<range_closure, Closure>(std::move(*this), std::move(closure));
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
constexpr static auto make_closure(Args&&... args) -> range_closure<Fn, std::decay_t<Args>...> {
    return range_closure<Fn, std::decay_t<Args>...>(std::forward<Args>(args)...);
}

// NOLINTBEGIN(cppcoreguidelines-require-return-statement)

template <typename Container, std::ranges::input_range Rng, typename... Args>
requires(!std::ranges::view<Container>)
[[nodiscard]] constexpr Container to(Rng&& range, Args&&... args) {
#ifdef CPP23
    return std::ranges::to<Container>(std::forward<Rng>(range), std::forward<Args>(args)...);
#else
    static_assert(!std::is_const_v<Container>, "Container must not be const.");
    static_assert(!std::is_volatile_v<Container>, "Container must not be volatile.");
    static_assert(std::is_class_v<Container>, "Container must be a class type.");

    if constexpr (ref_convertible<Rng, Container>) {
        if constexpr (std::constructible_from<Container, Rng, Args...>) {
            return Container(std::forward<Rng>(range), std::forward<Args>(args)...);
        }
        // there is no std::from_range_t in cpp20
        /* else if constexpr (std::constructible_from<
                               Container,
                               const std::from_range_t&,
                               Rng,
                               Args...>) {
            return Container(
                std::from_range, std::forward<Rng>(range), std::forward<Args>(args)...
            );
        } */
        else if constexpr (common_constructible<Rng, Container, Args...>) {
            return Container(
                std::ranges::begin(range), std::ranges::end(range), std::forward<Args>(args)...
            );
        }
        else if constexpr (constructible_appendable<Rng, Container, Args...>) {
            Container cnt(std::forward<Args>(args)...);
            if constexpr (std::ranges::sized_range<Rng> && std::ranges::sized_range<Container> &&
                          requires(
                              Container& cnt, const std::ranges::range_size_t<Container> count
                          ) {
                              cnt.reserve(count);
                              {
                                  cnt.capacity()
                              } -> std::same_as<std::ranges::range_size_t<Container>>;
                              {
                                  cnt.max_size()
                              } -> std::same_as<std::ranges::range_size_t<Container>>;
                          }) {
                cnt.reserve(
                    static_cast<std::ranges::range_size_t<Container>>(std::ranges::size(range))
                );
            }
            for (auto&& elem : range) {
                using element_type = decltype(elem);
                if constexpr (can_emplace_back<Container, element_type>) {
                    cnt.emplace_back(std::forward<element_type>(elem));
                }
                else if constexpr (can_push_back<Container, element_type>) {
                    cnt.push_back(std::forward<element_type>(elem));
                }
                else if constexpr (can_emplace_end<Container, element_type>) {
                    cnt.emplace(cnt.end(), std::forward<element_type>(elem));
                }
                else {
                    cnt.insert(cnt.end(), std::forward<element_type>(elem));
                }
            }
            return cnt;
        }
        else {
            static_assert(
                false,
                "ranges::to requires the result to be constructible from the source range, "
                "either by using a suitable constructor, or by inserting each element of the range "
                "into the default-constructed object."
            );
        }
    }
    else if constexpr (std::ranges::input_range<std::ranges::range_reference_t<Rng>>) {
        const auto form = []<typename Ty>(Ty&& elem) {
            return to<std::ranges::range_value_t<Container>>(std::forward<decltype(elem)>(elem));
        };
        return to<Container>(
            std::views::transform(std::ranges::ref_view{ range }, form), std::forward<Args>(args)...
        );
    }
    else {
        static_assert(
            false,
            "ranges::to requires the elements of the source range to be either implicitly "
            "convertible to the elements of the destination container, or be ranges themselves "
            "for ranges::to to be applied recursively."
        );
    }
#endif // CPP23
}

// NOLINTEND(cppcoreguidelines-require-return-statement)

namespace internal {

template <typename Container>
struct to_class_fn {
    template <std::ranges::input_range Rng, typename... Args>
    [[nodiscard]] constexpr auto operator()(Rng&& range, Args&&... args)
    requires requires {
        URANGES to<Container>(std::forward<Rng>(range), std::forward<Args>(args)...);
    }
    {
        return URANGES to<Container>(std::forward<Rng>(range), std::forward<Args>(args)...);
    }
};

} // namespace internal

template <typename Container, typename... Args>
[[nodiscard]] constexpr auto to(Args&&... args) {
#ifdef CPP23
    return std::ranges::to<Container>(std::forward<Args>(args)...);
#else
    return URANGES make_closure<URANGES internal::to_class_fn<Container>>(std::forward<Args>(args
    )...);
#endif
}

namespace internal {

// just for decltype
template <template <typename...> typename Cnt, typename Rng, typename... Args>
auto to_helper() {
    if constexpr (requires { Cnt(std::declval<Rng>(), std::declval<Args>()...); }) {
        return static_cast<decltype(Cnt(std::declval<Rng>(), std::declval<Args>()...))*>(nullptr);
    }
    // there is no std::from_range in cpp20
    /* else if constexpr (requires {
                           Cnt(std::from_range, std::declval<Rng>(), declval<Args>()...);
                       }) {
        return static_cast<
            decltype(Cnt(std::from_range, std::declval<Rng>(), std::declval<Args>()...))*>(nullptr);
    } */
    else if constexpr (requires {
                           Cnt(std::declval<UTILS phony_input_iterator<Rng>>(),
                               std::declval<UTILS phony_input_iterator<Rng>>(),
                               std::declval<Args>()...);
                       }) {
        return static_cast<decltype(Cnt(
            std::declval<UTILS phony_input_iterator<Rng>>(),
            std::declval<UTILS phony_input_iterator<Rng>>(),
            std::declval<Args>()...
        ))*>(nullptr);
    }
    else {
        static_assert(
            false,
            "No suitable way to deduce the type of the container, please provide more information "
            "about the container to use other 'to' function template."
        );
    }
}

} // namespace internal

template <
    template <typename...> typename Container,
    std::ranges::input_range Rng,
    typename... Args,
    typename Deduced =
        std::remove_pointer_t<decltype(URANGES internal::to_helper<Container, Rng, Args...>())>>
[[nodiscard]] constexpr auto to(Rng&& range, Args&&... args) -> Deduced {
#ifdef CPP23
    return std::ranges::to<Container>(std::forward<Rng>(range), std::forward<Args>(args)...);
#else
    return URANGES to<Deduced>(std::forward<Rng>(range), std::forward<Args>(args)...);
#endif
}

namespace internal {

template <template <typename...> typename Container>
struct to_template_fn {
    template <
        std::ranges::input_range Rng,
        typename... Args,
        typename Deduced =
            std::remove_pointer_t<decltype(URANGES internal::to_helper<Container, Rng, Args...>())>>
    [[nodiscard]] constexpr auto operator()(Rng&& range, Args&&... args) {
        return URANGES to<Deduced>(std::forward<Rng>(range), std::forward<Args>(args)...);
    }
};

} // namespace internal

// compiler and analyzer always deduce we are using another override
// so, changed the function name
template <template <typename...> typename Container, typename... Args>
[[nodiscard]] constexpr auto to_closure(Args&&... args) {
#ifdef CPP23
    return std::ranges::to<Container>(std::forward<Args>(args)...);
#else
    return URANGES make_closure<URANGES internal::to_template_fn<Container>>(std::forward<Args>(args
    )...);
#endif
}

} // namespace atom::utils::ranges
