#pragma once
#include <cstddef>

namespace atom::utils {

template <typename...>
struct type_list {};

template <std::size_t Index, typename TypeList>
struct type_list_element;

template <typename Ty, typename... Args>
struct type_list_element<0, type_list<Ty, Args...>> {
    using type = Ty;
};

template <std::size_t Index, typename Ty, typename... Args>
struct type_list_element<Index, type_list<Ty, Args...>> {
    using type = type_list_element<Index - 1, Args...>;
};

template <std::size_t Index, typename TypeList>
using type_list_element_t = typename type_list_element<Index, TypeList>::type;

} // namespace atom::utils
