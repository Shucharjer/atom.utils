#pragma once
#include <any>
#include <cstddef>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>
#include "core.hpp"

namespace atom::utils {

template <auto... Vals>
struct value_list {};

template <typename Ty>
struct value_list_size;
template <auto... Vals>
struct value_list_size<value_list<Vals...>> {
    constexpr static std::size_t value = sizeof...(Vals);
};
template <typename Ty>
constexpr size_t value_list_size_v = value_list_size<Ty>::value;

template <size_t Index, typename Ty>
struct value_list_element;
template <auto Val, auto... Others>
struct value_list_element<0, value_list<Val, Others...>> {
    constexpr static auto value = Val;
};
template <size_t Index, auto... Vals>
struct value_list_element<Index, value_list<Vals...>> {
    constexpr static auto value = value_list_element<Index - 1, value_list<Vals...>>::value;
};
template <size_t Index, typename Ty>
constexpr inline auto value_list_element_v = value_list_element<Index, Ty>::value;

/*! @cond TURN_OFF_DOXYGEN */

template <typename>
struct _is_value_list : std::false_type {};
template <auto... Vals>
struct _is_value_list<value_list<Vals...>> : std::true_type {};
template <typename Ty>
constexpr bool _is_value_list_v = _is_value_list<Ty>::value;

/*! @endcond */

/**
 * @brief Make a tuple from a value_list.
 */
template <typename Ty>
requires _is_value_list_v<Ty>
constexpr auto make_tuple() noexcept {
    return []<size_t... Is>(std::index_sequence<Is...>) {
        return std::make_tuple(value_list_element_v<Is, Ty>...);
    }(std::make_index_sequence<value_list_size_v<Ty>>());
}
/**
 * @brief Make a tuple from a value_list.
 */
template <typename Ty>
requires _is_value_list_v<Ty>
constexpr auto make_tuple(Ty) noexcept {
    return []<size_t... Is>(std::index_sequence<Is...>) {
        return std::make_tuple(value_list_element_v<Is, Ty>...);
    }(std::make_index_sequence<value_list_size_v<Ty>>());
}

/*! @cond TURN_OFF_DOXYGEN */

struct _poly_empty_impl {
    template <size_t Index, typename... Args>
    constexpr inline auto invoke(Args&&... args) noexcept {}
    template <size_t Index, typename... Args>
    constexpr inline auto invoke(Args&&... args) const noexcept {}
};

template <typename Ty>
concept _poly_object = requires {
    // declared a class template called interface.
    typename Ty::template interface<_poly_empty_impl>;
    std::is_base_of_v<_poly_empty_impl, typename Ty::template interface<_poly_empty_impl>>;

    // declared a value_list called interface.
    typename Ty::template impl<typename Ty::template interface<_poly_empty_impl>>;
    _is_value_list_v<typename Ty::template impl<typename Ty::template interface<_poly_empty_impl>>>;
};

template <typename Ty, typename Object>
concept _poly_impl = requires {
    _poly_object<Object>;
    typename Object::template impl<std::remove_cvref_t<Ty>>;
    _is_value_list_v<typename Object::template impl<std::remove_cvref_t<Ty>>>;
};

template <typename>
struct _mem_func_traits;
template <typename Ret, typename Class, typename... Args>
struct _mem_func_traits<Ret (Class::*)(Args...)> {
    using type        = Ret (*)(Class&, Args...);
    using return_type = Ret;
    using class_type  = Class;
    using args_type   = std::tuple<Args...>;
    using static_type = Ret (*)(void*, Args...);
};
template <typename Ret, typename Class, typename... Args>
struct _mem_func_traits<Ret (Class::*)(Args...) const> {
    using type        = Ret (*)(const Class&, Args...);
    using return_type = Ret;
    using class_type  = Class;
    using args_type   = std::tuple<Args...>;
    using static_type = Ret (*)(const void*, Args...);
};

template <_poly_object Object>
struct vtable {
    using _empty_interface     = typename Object::template interface<_poly_empty_impl>;
    using _empty_impl          = typename Object::template impl<_empty_interface>;
    constexpr static auto size = value_list_size_v<_empty_impl>;

    template <typename MemFunc>
    using static_type_t = typename _mem_func_traits<MemFunc>::static_type;

    template <auto... Vals>
    constexpr static auto _deduce(value_list<Vals...>) noexcept {
        return static_cast<std::tuple<static_type_t<decltype(Vals)>...>*>(nullptr);
    }
    constexpr static auto _deduce() noexcept { return _deduce(_empty_impl{}); }

    using type = std::remove_pointer_t<decltype(_deduce())>;
};
template <_poly_object Object>
using vtable_t = typename vtable<Object>::type;

/*! @cond TURN_OFF_DOXYGEN */

template <_poly_object Object, _poly_impl<Object> Impl>
struct _vtable_value_list {
    template <typename ValueList>
    struct _static_list;

    template <auto... Vals>
    struct _static_list<value_list<Vals...>> {
        template <auto, typename>
        struct _element;
        template <auto Val, typename Ret, typename Class, typename... Args>
        struct _element<Val, Ret (Class::*)(Args...)> {
            constexpr static Ret (*value)(any&, Args...) = [](void* ptr, Args... args) {
                return (static_cast<Class*>(ptr)->*Val)(std::forward<Args>(args)...);
            };
        };
        template <auto Val, typename Ret, typename Class, typename... Args>
        struct _element<Val, Ret (Class::*)(Args...) const> {
            constexpr static Ret (*value)(const void*, Args...) = [](const void* ptr,
                                                                     Args... args) {
                return (static_cast<const Class*>(ptr)->*Val)(std::forward<Args>(args)...);
            };
        };

        using type = value_list<_element<Vals, decltype(Vals)>::value...>;
    };

    using type = typename _static_list<typename Object::template impl<Impl>>::type;
};

template <_poly_object Object, _poly_impl<Object> Impl>
using _vtable_value_list_t = _vtable_value_list<Object, Impl>::type;

/*! @endcond */

template <_poly_object Object, _poly_impl<Object> Impl>
consteval inline auto make_vtable_tuple() noexcept {
    return make_tuple<_vtable_value_list_t<Object, Impl>>();
}

template <_poly_object Object, _poly_impl<Object> Impl>
constexpr static inline auto static_vtable_tuple = make_vtable_tuple<Object, Impl>();

template <_poly_object Object, _poly_impl<Object> Impl>
consteval static inline auto make_vtable_array() {
    constexpr auto vtable = make_vtable_tuple<Object, Impl>();
    std::array<void*, std::tuple_size_v<decltype(vtable)>> array;
    [&]<size_t... Is>(std::index_sequence<Is...>) {
        ((array[Is] = std::get<Is>(vtable)), ...);
    }(std::make_index_sequence<std::tuple_size_v<decltype(vtable)>>{});
    return array;
}

template <_poly_object Object, _poly_impl<Object> Impl>
constexpr static inline auto static_vtable_array = make_vtable_array<Object, Impl>();

template <typename Poly>
class polymorphic_base;

// polymorphic<Object> ->  Object::temlate interface<_impl_> -> polymorphic_base<polymorphic<Object>

/**
 * @brief Static polymorphic object.
 * @tparam Object The object type that provides the interface and implementation.
 * @tparam Size The size of the storage for the polymorphic object. Default is
 * `k_default_any_storege_size`.
 * @tparam Align The alignment of the storage for the polymorphic object. Default is
 * `k_default_any_storege_size`.
 * @tparam Ops The operations to be used for the polymorphic object. Default is a tuple containing
 * `any_operation_copy_construct`.
 */
template <
    _poly_object Object, size_t Size = k_default_any_storege_size,
    size_t Align = k_default_any_storege_size,
    typename Ops = std::tuple<any_operation_copy_construct>>
class polymorphic
    : private Object::template interface<polymorphic_base<polymorphic<Object, Size, Align, Ops>>> {

    friend class polymorphic_base<polymorphic>;

    template <typename Base>
    using interface_t = typename Object::template interface<Base>;

    using interface = interface_t<polymorphic_base<polymorphic>>;

    /// @brief The storage for the polymorphic object.
    basic_any<Size, Align, Ops> any_;

    using vtable                                     = vtable_t<Object>;
    constexpr static size_t _store_vtable_as_pointer = sizeof(vtable) > sizeof(vtable*);

    /// @brief The vtable for the polymorphic object.
    std::conditional_t<_store_vtable_as_pointer, const vtable*, vtable> vtable_;

    template <_poly_impl<Object> Impl>
    consteval static auto _vtable_value() noexcept {
        if constexpr (_store_vtable_as_pointer) {
            return &static_vtable_tuple<Object, Impl>;
        }
        else {
            return make_vtable_tuple<Object, Impl>();
        }
    }

public:
    constexpr polymorphic() noexcept
        : vtable_(make_vtable_tuple<Object, interface_t<_poly_empty_impl>>()), any_() {}

    template <_poly_impl<Object> Impl>
    constexpr polymorphic(Impl&& impl)
        : vtable_(_vtable_value<Impl>()), any_(std::forward<Impl>(impl)) {}

    constexpr inline interface* operator->() noexcept { return this; }
    constexpr inline const interface* operator->() const noexcept { return this; }
};

template <
    _poly_object Object, _poly_impl<Object> Impl, size_t Size = k_default_any_storege_size,
    size_t Align = k_default_any_storege_size, typename Ops = std::tuple<>, typename... Args>
requires std::constructible_from<Impl, Args...>
constexpr inline polymorphic<Object, Size, Align, Ops> make_polymorphic(Args&&... args) {
    // TODO:
}

template <typename Poly>
class polymorphic_base;

template <_poly_object Object, size_t Size, size_t Align, typename Ops>
class polymorphic_base<polymorphic<Object, Size, Align, Ops>> {
public:
    using polymorphic_type = polymorphic<Object, Size, Align, Ops>;

    template <size_t Index, typename... Args>
    constexpr inline decltype(auto) invoke(Args&&... args) {
        auto* const ptr = static_cast<polymorphic_type*>(this);
        if constexpr (ptr->_store_vtable_as_pointer) {
            return std::get<Index>(*(ptr->vtable_))(ptr->any_.get(), std::forward<Args>(args)...);
        }
        else {
            return std::get<Index>(ptr->vtable_)(ptr->any_.get(), std::forward<Args>(args)...);
        }
    }

    template <size_t Index, typename... Args>
    constexpr inline decltype(auto) invoke(Args&&... args) const {
        auto* const ptr = static_cast<const polymorphic_type*>(this);
        if constexpr (ptr->_store_vtable_as_pointer) {
            return std::get<Index>(*(ptr->vtable_))(ptr->any_.get(), std::forward<Args>(args)...);
        }
        else {
            return std::get<Index>(ptr->vtable_)(ptr->any_.get(), std::forward<Args>(args)...);
        }
    }
};

} // namespace atom::utils
