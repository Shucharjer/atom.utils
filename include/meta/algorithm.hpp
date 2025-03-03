#pragma once
#include "meta.hpp"
#include "meta/sequence.hpp"

namespace atom::utils {

template <auto lhs, auto rhs>
struct less {
    constexpr static bool value = lhs < rhs;
};

template <auto lhs, auto rhs>
struct equal {
    constexpr static bool value = lhs == rhs;
};

template <auto lhs, auto rhs>
struct greater {
    constexpr static bool value = lhs > rhs;
};

template <
    bool Cond, typename First, typename Second, template <typename, typename> typename Operator>
struct operate_if {
    using type = std::conditional_t<Cond, typename Operator<First, Second>::type, First>;
};

template <
    bool Cond, typename First, typename Second, template <typename, typename> typename Operator>
using operate_if_t = typename operate_if<Cond, First, Second, Operator>::type;

/*! @cond TURN_OFF_DOXYGEN */
namespace internal {

template <typename Seq, template <auto> typename Pr, typename Current = empty_sequence_t<Seq>>
struct filter_impl;

template <
    typename Elem, template <auto> typename Pr, template <typename, auto...> typename Sequence,
    typename Previous>
struct filter_impl<Sequence<Elem>, Pr, Previous> {
    using type = Previous;
};

template <
    typename Elem, Elem... vals, template <auto> typename Pr,
    template <typename, auto...> typename Sequence, typename Previous>
struct filter_impl<Sequence<Elem, vals...>, Pr, Previous> {
    using sequence                = Sequence<Elem, vals...>;
    constexpr static auto current = front_v<sequence>;
    using result =
        operate_if_t<Pr<current>::value, Previous, Sequence<Elem, current>, concat_sequence>;
    using type = typename filter_impl<pop_front_t<sequence>, Pr, result>::type;
};

} // namespace internal
/*! @endcond */

template <typename Seq, template <auto> typename Pr>
struct filt {
    using type = internal::filter_impl<Seq, Pr>::type;
};

template <typename Seq, template <auto> typename Pr>
using filt_t = typename filt<Seq, Pr>::type;

template <typename Seq, template <auto> typename Pr>
struct filt_not {
    template <auto val>
    struct not_ {
        constexpr static bool value = !Pr<val>::value;
    };
    using type = filt_t<Seq, not_>;
};

template <typename Seq, template <auto> typename Pr>
using filt_not_t = typename filt_not<Seq, Pr>::type;

template <typename Seq, template <auto, auto> typename Compare = less>
using quick_sort_t = typename quick_sort<Seq, Compare>::type;

template <
    typename Elem, template <auto, auto> typename Compare,
    template <typename, auto...> typename Sequence>
struct quick_sort<Sequence<Elem>, Compare> {
    using type = Sequence<Elem>;
};

template <
    typename Elem, Elem val, template <auto, auto> typename Compare,
    template <typename, auto...> typename Sequence>
struct quick_sort<Sequence<Elem, val>, Compare> {
    using type = Sequence<Elem, val>;
};

template <
    typename Elem, Elem... vals, template <auto, auto> typename Compare,
    template <typename, auto...> typename Sequence>
requires(sizeof...(vals) > 1)
struct quick_sort<Sequence<Elem, vals...>, Compare> {
private:
    using sequence              = pop_front_t<Sequence<Elem, vals...>>;
    constexpr static auto pivot = front_v<Sequence<Elem, vals...>>;

    template <auto val>
    struct compare {
        constexpr static bool value = Compare<val, pivot>::value;
    };

    using left  = filt_t<sequence, compare>;
    using right = filt_not_t<sequence, compare>;

public:
    using type = concat_sequence_t<
        quick_sort_t<left, Compare>,
        concat_sequence_t<Sequence<Elem, pivot>, quick_sort_t<right, Compare>>>;
};

template <
    template <typename...> typename Cnt, typename Elem, Elem... vals,
    template <typename, auto...> typename Sequence>
struct as_container<Sequence<Elem, vals...>, Cnt> {
    constexpr static auto value = Cnt<Elem>{ vals... };
};

template <typename Seq, template <typename...> typename Cnt = std::initializer_list>
constexpr auto as_container_v = as_container<Seq, Cnt>::value;

} // namespace atom::utils
