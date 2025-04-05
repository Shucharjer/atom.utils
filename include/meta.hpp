#pragma once
#include <cstddef>
#include <initializer_list>

namespace atom::utils {

template <typename Elem, Elem...>
struct sequence;

template <typename Seq>
struct sequence_element;

template <typename Seq>
struct sequence_size;

template <typename, typename>
struct concat_sequence;

template <typename, auto>
struct append_sequence;

template <typename, typename>
struct merge_sequence;

template <std::size_t N, typename Ty = std::size_t>
struct integer_seq;

template <typename, typename>
struct remake_sequence;

template <typename Seq>
struct empty_sequence;

template <auto lhs, auto rhs>
struct less;

template <auto lhs, auto rhs>
struct equal;

template <auto lhs, auto rhs>
struct greater;

template <
    bool Cond, typename First, typename Second, template <typename, typename> typename Operator>
struct operate_if;

template <typename Seq, template <auto> typename Pr>
struct filt;

template <typename Seq, template <auto> typename Pr>
struct filt_not;

template <typename, template <auto, auto> typename Compare = less>
struct quick_sort;

template <typename Seq, template <typename...> typename Cnt = std::initializer_list>
struct as_container;

/**
 * @brief Return whether a expression is const evaluated.
 * @note If you want to know whether the function could run at compile-time, please call
 * std::is_const_evaluated()
 * @bug Could not verify whether a callable is a constexpr.
 */
template <auto expr>
consteval bool is_constexpr();

} // namespace atom::utils
