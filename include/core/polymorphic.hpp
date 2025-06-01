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
    _poly_empty_impl() = delete;
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

template <typename, typename>
struct _replace_caller;
template <typename Ty, typename Ret, typename Class, typename... Args>
struct _replace_caller<Ty, Ret (*)(Class&, Args...)> {
    using type = Ret (*)(Ty, Args...);
};
template <typename Ty, typename Ret, typename Class, typename... Args>
struct _replace_caller<Ty, Ret (*)(Class*, Args...)> {
    using type = Ret (*)(Ty, Args...);
};
template <typename Ty, typename Ret, typename Class, typename... Args>
struct _replace_caller<Ty*, Ret (*)(Class&, Args...)> {
    using type = Ret (*)(Ty*, Args...);
};
template <typename Ty, typename Ret, typename Class, typename... Args>
struct _replace_caller<Ty*, Ret (*)(Class*, Args...)> {
    using type = Ret (*)(Ty*, Args...);
};
template <typename Ty, typename Ret, typename Class, typename... Args>
struct _replace_caller<Ty, Ret (*)(const Class&, Args...)> {
    using type = Ret (*)(const Ty&, Args...);
};
template <typename Ty, typename Ret, typename Class, typename... Args>
struct _replace_caller<Ty, Ret (*)(const Class*, Args...)> {
    using type = Ret (*)(const Ty&, Args...);
};
template <typename Ty, typename Ret, typename Class, typename... Args>
struct _replace_caller<Ty*, Ret (*)(const Class&, Args...)> {
    using type = Ret (*)(const Ty*, Args...);
};
template <typename Ty, typename Ret, typename Class, typename... Args>
struct _replace_caller<Ty*, Ret (*)(const Class*, Args...)> {
    using type = Ret (*)(const Ty*, Args...);
};
template <typename Ty, typename Static>
using _replace_caller_t = typename _replace_caller<Ty, Static>::type;

template <typename Ty>
struct _ref_non_pointer {
    using type = Ty&;
};
template <typename Ty>
struct _ref_non_pointer<Ty*> {
    using type = Ty*;
};
template <typename Ty>
using _ref_non_pointer_t = typename _ref_non_pointer<Ty>::type;

template <typename Ty>
struct _cref_non_pointer {
    using type = const Ty&;
};
template <typename Ty>
struct _cref_non_pointer<const Ty*> {
    using type = const Ty*;
};
template <typename Ty>
using _cref_non_pointer_t = typename _cref_non_pointer<Ty>::type;

/*! @endcond */

template <_poly_object Object, typename Any = void*>
struct vtable {
    using _empty_interface     = typename Object::template interface<_poly_empty_impl>;
    using _empty_impl          = typename Object::template impl<_empty_interface>;
    constexpr static auto size = value_list_size_v<_empty_impl>;

    template <typename MemFunc>
    using static_type_t = _replace_caller_t<Any, typename _mem_func_traits<MemFunc>::static_type>;

    template <auto... Vals>
    constexpr static auto _deduce(value_list<Vals...>) noexcept {
        return static_cast<std::tuple<static_type_t<decltype(Vals)>...>*>(nullptr);
    }
    constexpr static auto _deduce() noexcept { return _deduce(_empty_impl{}); }

    using type = std::remove_pointer_t<decltype(_deduce())>;
    type value;

    template <size_t Index, typename... Args>
    constexpr decltype(auto) invoke(_ref_non_pointer_t<Any> any, Args&&... args) const {
        return std::get<Index>(value)(any, std::forward<Args>(args)...);
    }

    template <size_t Index, typename... Args>
    constexpr decltype(auto) invoke_const(_cref_non_pointer_t<Any> any, Args&&... args) const {
        return std::get<Index>(value)(any, std::forward<Args>(args)...);
    }
};

/*! @cond TURN_OFF_DOXYGEN */

template <typename ValueList, typename Any>
struct _static_list;

template <auto... Vals, size_t Size, size_t Align, typename Ops>
struct _static_list<value_list<Vals...>, basic_any<Size, Align, Ops>> {

    using Any = basic_any<Size, Align, Ops>;

    template <auto, typename>
    struct _element;
    template <auto Val, typename Ret, typename Class, typename... Args>
    struct _element<Val, Ret (Class::*)(Args...)> {
        constexpr static Ret (*value)(Any&, Args...) = [](Any& any, Args... args) {
            return (any_cast<Class*>(any)->*Val)(std::forward<Args>(args)...);
        };
    };

    template <auto Val, typename Ret, typename Class, typename... Args>
    struct _element<Val, Ret (Class::*)(Args...) const> {
        constexpr static Ret (*value)(const Any&, Args...) = [](const Any& any, Args... args) {
            return (any_cast<const Class*>(any)->*Val)(std::forward<Args>(args)...);
        };
    };

    using type = value_list<_element<Vals, decltype(Vals)>::value...>;
};

template <auto... Vals>
struct _static_list<value_list<Vals...>, void*> {

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
        constexpr static Ret (*value)(const void*, Args...) = [](const void* ptr, Args... args) {
            return (static_cast<const Class*>(ptr)->*Val)(std::forward<Args>(args)...);
        };
    };

    using type = value_list<_element<Vals, decltype(Vals)>::value...>;
};
template <typename ValueList, typename Any>
using _static_list_t = _static_list<ValueList, Any>::type;

template <_poly_object Object, _poly_impl<Object> Impl, typename Any>
using _vtable_value = _static_list_t<typename Object::template impl<Impl>, Any>;

template <_poly_object Object, _poly_impl<Object> Impl, typename Any>
consteval auto _vtable_tuple_value() noexcept {
    using _vtable_value = _vtable_value<Object, Impl, Any>;
    return make_tuple<_vtable_value>();
}

template <_poly_object Object, _poly_impl<Object> Impl, typename Any>
constexpr static auto _static_vtable =
    vtable<Object, Any>{ .value = _vtable_tuple_value<Object, Impl, Any>() };

/*! @endcond */

template <typename Poly>
class polymorphic_base;

/*
polymorphic<Object> ->  Object::temlate interface<_impl_> -> polymorphic_base<polymorphic<Object>
*/

template <
    _poly_object Object, size_t Size = k_default_any_storege_size,
    size_t Align = k_default_any_storege_size,
    typename Ops = std::tuple<any_operation_copy_construct>>
class polymorphic
    : private Object::template interface<polymorphic_base<polymorphic<Object, Size, Align, Ops>>> {

    friend class polymorphic_base<polymorphic>;

    using interface = typename Object::template interface<polymorphic_base<polymorphic>>;

    using any = basic_any<Size, Align, Ops>;

    any any_;
    vtable<Object, any> vtable_;

public:
    constexpr polymorphic() noexcept
        : vtable_(_vtable_tuple_value<
                  Object, typename Object::template interface<_poly_empty_impl>, any>()),
          any_() {}

    template <_poly_impl<Object> Impl>
    constexpr polymorphic(Impl&& impl)
        : vtable_(_vtable_tuple_value<Object, Impl, any>()), any_(std::forward<Impl>(impl)) {}

    interface* operator->() noexcept { return this; }
    const interface* operator->() const noexcept { return this; }
};

template <typename Poly>
class polymorphic_base;

template <_poly_object Object, size_t Size, size_t Align, typename Ops>
class polymorphic_base<polymorphic<Object, Size, Align, Ops>> {
public:
    using polymorphic_type = polymorphic<Object, Size, Align, Ops>;

    template <size_t Index, typename... Args>
    constexpr decltype(auto) invoke(Args&&... args) {
        auto* const ptr = static_cast<polymorphic_type*>(this);
        return ptr->vtable_.template invoke<Index>(ptr->any_, std::forward<Args>(args)...);
    }

    template <size_t Index, typename... Args>
    constexpr decltype(auto) invoke(Args&&... args) const {
        auto* const ptr = static_cast<const polymorphic_type*>(this);
        return ptr->vtable_.template invoke_const<Index>(ptr->any_, std::forward<Args>(args)...);
    }
};

template <size_t Index, typename... Args>
constexpr inline auto call_polymorphic(_poly_empty_impl* empty) noexcept {}

template <size_t Index, typename... Args>
constexpr inline auto call_polymorphic(const _poly_empty_impl* empty) noexcept {}

/**
 * @brief
 *
 * @tparam Index
 * @tparam Self {$1:unknown}::interface template<polymorphic_base<polymorphic<$1>>>
 * @tparam Args
 * @param self
 * @param args
 * @return constexpr auto
 */
template <size_t Index, typename Object, size_t Size, size_t Align, typename Ops, typename... Args>
constexpr inline auto call_polymorphic(
    polymorphic_base<polymorphic<Object, Size, Align, Ops>>* self, Args&&... args) noexcept {
    self->template invoke<Index>(std::forward<Args>(args)...);
}

template <size_t Index, typename Object, size_t Size, size_t Align, typename Ops, typename... Args>
constexpr inline auto call_polymorphic(
    const polymorphic_base<polymorphic<Object, Size, Align, Ops>>* self, Args&&... args) noexcept {
    self->template invoke<Index>(std::forward<Args>(args)...);
}

} // namespace atom::utils
