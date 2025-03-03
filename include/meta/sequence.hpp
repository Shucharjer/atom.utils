#pragma once
#include <utility>
#include "meta.hpp"

namespace atom::utils {

template <typename Elem, Elem...>
struct sequence {};

template <typename Elem, Elem... vals, template <typename, auto...> typename Sequence>
struct sequence_element<Sequence<Elem, vals...>> {
    using type = Elem;
};

template <typename Seq>
using sequence_element_t = typename sequence_element<Seq>::type;

template <typename Elem, Elem... vals, template <typename, auto...> typename Sequence>
struct sequence_size<Sequence<Elem, vals...>> {
    constexpr static std::size_t value = sizeof...(vals);
};

template <typename Seq>
constexpr inline auto sequence_size_v = sequence_size<Seq>::value;

template <typename Elem, Elem... S1, Elem... S2, template <typename, auto...> typename Sequence>
struct concat_sequence<Sequence<Elem, S1...>, Sequence<Elem, S2...>> {
    using type = Sequence<Elem, S1..., S2...>;
};

template <typename Seq1, typename Seq2>
using concat_sequence_t = typename concat_sequence<Seq1, Seq2>::type;

template <typename Elem, Elem... Is, Elem Val, template <typename, auto...> typename Sequence>
struct append_sequence<Sequence<Elem, Is...>, Val> {
    using type = Sequence<Elem, Is..., Val>;
};

template <typename Seq, auto Val>
using append_sequence_t = typename append_sequence<Seq, Val>::type;

template <typename Elem, Elem... S1, Elem... S2, template <typename, auto...> typename Sequence>
struct merge_sequence<Sequence<Elem, S1...>, Sequence<Elem, S2...>> {
    using type = Sequence<Elem, S1..., (sizeof...(S1) + S2)...>;
};

template <typename Seq1, typename Seq2>
using merge_sequence_t = typename merge_sequence<Seq1, Seq2>::type;

template <typename Ty>
struct integer_seq<0U, Ty> {
    using type = std::integer_sequence<Ty>;
};

template <std::size_t N, typename Ty>
struct integer_seq {
    using type = append_sequence_t<typename integer_seq<N - 1, Ty>::type, N - 1>;
};

template <std::size_t N, typename Ty = std::size_t>
using integer_seq_t = typename integer_seq<N, Ty>::type;

template <typename Elem, Elem... S1, Elem... S2, template <typename, auto...> typename Sequence>
struct remake_sequence<Sequence<Elem, S1...>, Sequence<Elem, S2...>> {
    using type = integer_seq_t<sizeof...(S1) + sizeof...(S2)>;
};

template <typename Seq1, typename Seq2>
using remake_sequence_t = typename remake_sequence<Seq1, Seq2>::type;

template <typename Seq>
struct front;

template <typename Elem, Elem head, Elem... tail, template <typename, auto...> typename Sequence>
struct front<Sequence<Elem, head, tail...>> {
    constexpr static auto value = head;
};

template <typename Seq>
constexpr auto front_v = front<Seq>::value;

template <typename Seq>
struct pop_front;

template <typename Elem, Elem head, Elem... tail, template <typename, auto...> typename Sequence>
struct pop_front<Sequence<Elem, head, tail...>> {
    using type = Sequence<Elem, tail...>;
};

template <typename Seq>
using pop_front_t = typename pop_front<Seq>::type;

template <typename Elem, Elem... vals, template <typename, auto...> typename Sequence>
struct empty_sequence<Sequence<Elem, vals...>> {
    using type = Sequence<Elem>;
};

template <typename Seq>
using empty_sequence_t = typename empty_sequence<Seq>::type;

} // namespace atom::utils
