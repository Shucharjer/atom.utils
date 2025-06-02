#pragma once
#include <cstddef>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>

namespace atom::utils {

template <typename, typename>
struct has_type : std::false_type {};
template <typename Ty, typename... Types>
struct has_type<Ty, std::tuple<Types...>> {
    constexpr static bool value = (std::is_same_v<Ty, Types> || ...);
};
template <typename Ty, typename Tuple>
constexpr bool has_type_v = has_type<Ty, Tuple>::value;

template <typename Ty, typename Tuple>
struct tuple_first;
template <typename Ty, typename... Types>
struct tuple_first<Ty, std::tuple<Types...>> {
    constexpr static size_t value = []<size_t... Is>(std::index_sequence<Is...>) {
        auto index = static_cast<size_t>(-1);
        (..., (index = (std::is_same_v<Ty, Types> ? Is : index)));
        return index;
    }(std::index_sequence_for<Types...>());
    static_assert(value < sizeof...(Types), "Type not found in tuple");
};
template <typename Ty, typename Tuple>
constexpr size_t tuple_first_v = tuple_first<Ty, Tuple>::value;

struct poly_op_copy_construct {
    void (*value)(void* ptr, const void* other) = nullptr;
};

struct poly_op_move_construct {
    void (*value)(void* ptr, void* other) = nullptr;
};

struct poly_op_copy_assign {
    void (*value)(void* ptr, const void* other) = nullptr;
};

struct poly_op_move_assign {
    void (*value)(void* ptr, void* other) = nullptr;
};

constexpr size_t k_default_poly_storage_size = 16;

constexpr size_t k_default_poly_storage_align = 16;

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
template <size_t Index, auto Val, auto... Others>
struct value_list_element<Index, value_list<Val, Others...>> {
    constexpr static auto value = value_list_element<Index - 1, value_list<Others...>>::value;
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

#ifdef __GNUC__
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wreturn-type"
#elif defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wreturn-type"
#elif defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable : 4715)
#endif
struct _poly_empty_impl {
    struct _universal {
        template <typename Ty>
        constexpr operator Ty&&() {}
    };

    template <size_t Index, typename... Args>
    constexpr inline auto invoke(Args&&... args) noexcept {
        return _universal{};
    }
    template <size_t Index, typename... Args>
    constexpr inline auto invoke(Args&&... args) const noexcept {
        return _universal{};
    }
};
#ifdef __GNUC__
    #pragma GCC diagnostic pop
#elif defined(__clang__)
    #pragma clang diagnostic pop
#elif defined(_MSC_VER)
    #pragma warning(pop)
#endif

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
            constexpr static Ret (*value)(void*, Args...) = [](void* ptr, Args... args) {
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
        template <auto Val, typename Func>
        constexpr static auto _element_v = _element<Val, Func>::value;

        using type = value_list<_element_v<Vals, decltype(Vals)>...>;
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

template <typename Poly>
class polymorphic_base;

// polymorphic<Object> ->  Object::temlate interface<Impl> -> polymorphic_base<polymorphic<Object>

/**
 * @brief Static polymorphic object.
 * This class provides a way to create a high performance polymorphic object **quickly**, but if you
 * want to get higher performance, you should use a builtin vtable instead of this class.
 *
 * @tparam Object The object type that provides the interface and implementation.
 * @tparam Size The size of the storage for the polymorphic object. Default is
 * `k_default_poly_storege_size`.
 * @tparam Align The alignment of the storage for the polymorphic object. Default is
 * `k_default_poly_storege_size`.
 * @tparam Ops The operations to be used for the polymorphic object. Default is a empty tuple.
 */
template <
    _poly_object Object, size_t Size = k_default_poly_storage_size,
    size_t Align = k_default_poly_storage_align, typename Ops = std::tuple<>>
class polymorphic
    : private Object::template interface<polymorphic_base<polymorphic<Object, Size, Align, Ops>>> {

    friend class polymorphic_base<polymorphic>;

    using _empty_interface = typename Object::template interface<_poly_empty_impl>;

    /// The pointer to the polymorphic object.
    void* ptr_;

    struct _poly_operation_destroy {
        void (*value)(void* ptr) = nullptr;
    };
    using ops_t = decltype(std::tuple_cat(Ops{}, std::tuple<_poly_operation_destroy>{}));
    /// @brief Operations for the polymorphic object.
    ops_t operations_;

    using vtable                                     = vtable_t<Object>;
    constexpr static size_t _store_vtable_as_pointer = sizeof(vtable) > sizeof(void*) * 2;

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

    /// @brief Storage for the polymorphic object.
    alignas(Align) std::byte storage_[Size];

    template <_poly_impl<Object> Impl>
    constexpr static bool builtin_storable = sizeof(Impl) <= Size && alignof(Impl) <= Align;

    template <_poly_impl<Object> Impl>
    constexpr void* _init_ptr() {
        if constexpr (builtin_storable<Impl>) {
            return static_cast<void*>(storage_);
        }
        else {
            return operator new(sizeof(Impl), std::align_val_t(Align));
        }
    }

    template <_poly_impl<Object> Impl>
    constexpr void construct(Impl&& impl) {
        ::new (ptr_) Impl(std::forward<Impl>(impl));
        ptr_ = std::launder(static_cast<Impl*>(ptr_));
    }

    constexpr void destroy() {
        std::get<tuple_first_v<_poly_operation_destroy, ops_t>>(operations_).value(ptr_);
    }

    template <_poly_impl<Object> Impl>
    constexpr static inline ops_t _operations() noexcept {
        ops_t ops;
        if constexpr (has_type_v<poly_op_copy_construct, ops_t>) {
            std::get<tuple_first_v<poly_op_copy_construct, ops_t>>(ops).value =
                [](void* lhs,
                   const void* rhs) noexcept(std::is_nothrow_copy_constructible_v<Impl>) {
                    ::new (lhs) Impl(*static_cast<const Impl*>(rhs));
                };
        }
        if constexpr (has_type_v<poly_op_move_construct, ops_t>) {
            std::get<tuple_first_v<poly_op_move_construct, ops_t>>(ops).value =
                [](void* lhs, void* rhs) noexcept(std::is_nothrow_move_constructible_v<Impl>) {
                    ::new (lhs) Impl(std::move(*static_cast<Impl*>(rhs)));
                };
        }
        if constexpr (has_type_v<poly_op_copy_assign, ops_t>) {
            std::get<tuple_first_v<poly_op_copy_assign, ops_t>>(ops).value =
                [](void* lhs, const void* rhs) noexcept(std::is_nothrow_copy_assignable_v<Impl>) {
                    *static_cast<Impl*>(lhs) = *static_cast<const Impl*>(rhs);
                };
        }
        if constexpr (has_type_v<poly_op_move_assign, ops_t>) {
            std::get<tuple_first_v<poly_op_move_assign, ops_t>>(ops).value =
                [](void* lhs, void* rhs) noexcept(std::is_nothrow_move_assignable_v<Impl>) {
                    *static_cast<Impl*>(lhs) = std::move(*static_cast<Impl*>(rhs));
                };
        }
        std::get<tuple_first_v<_poly_operation_destroy, ops_t>>(ops).value =
            [](void* ptr) noexcept(std::is_nothrow_destructible_v<Impl>) {
                static_cast<Impl*>(ptr)->~Impl();
            };
        return ops;
    }

public:
    using interface = typename Object::template interface<polymorphic_base<polymorphic>>;

    /**
     * @brief Constructs a polymorphic object with an empty interface.
     * @warning This constructor initializes the polymorphic object with an empty interface. You
     * should construct the polymorphic object with a valid implementation before using it.
     */
    constexpr polymorphic() noexcept
        : ptr_(nullptr), vtable_(_vtable_value<_empty_interface>()), storage_(), operations_() {}

    template <_poly_impl<Object> Impl>
    constexpr polymorphic(Impl&& impl)
        : ptr_(_init_ptr<Impl>()), vtable_(_vtable_value<Impl>()), storage_(),
          operations_(_operations<Impl>()) {
        construct(std::forward<Impl>(impl));
    }

    polymorphic(const polymorphic& that)
    requires has_type_v<poly_op_copy_construct, ops_t>
    {}

    polymorphic(polymorphic&& that)
    requires has_type_v<poly_op_move_construct, ops_t>
    {}

    polymorphic& operator=(const polymorphic& that)
    requires has_type_v<poly_op_copy_assign, ops_t>
    {}

    polymorphic& operator=(polymorphic&& that)
    requires has_type_v<poly_op_move_assign, ops_t>
    {}

    constexpr ~polymorphic() {
        if (ptr_) [[likely]] {
            destroy();
            ptr_ = nullptr;
        }
    }

    [[nodiscard]] constexpr inline interface* operator->() noexcept { return this; }
    [[nodiscard]] constexpr inline const interface* operator->() const noexcept { return this; }

    [[nodiscard]] constexpr inline operator bool() const noexcept { return ptr_; }

    [[nodiscard]] constexpr inline void* data() noexcept { return ptr_; }
    [[nodiscard]] constexpr inline const void* data() const noexcept { return ptr_; }
};

template <
    _poly_object Object, _poly_impl<Object> Impl, size_t Size = k_default_poly_storage_size,
    size_t Align = k_default_poly_storage_align, typename Ops = std::tuple<>, typename... Args>
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
        if constexpr (polymorphic_type::_store_vtable_as_pointer) {
            return std::get<Index>(*(ptr->vtable_))(ptr->ptr_, std::forward<Args>(args)...);
        }
        else {
            return std::get<Index>(ptr->vtable_)(ptr->ptr_, std::forward<Args>(args)...);
        }
    }

    template <size_t Index, typename... Args>
    constexpr inline decltype(auto) invoke(Args&&... args) const {
        auto* const ptr = static_cast<const polymorphic_type*>(this);
        if constexpr (polymorphic_type::_store_vtable_as_pointer) {
            return std::get<Index>(*(ptr->vtable_))(ptr->ptr_, std::forward<Args>(args)...);
        }
        else {
            return std::get<Index>(ptr->vtable_)(ptr->ptr_, std::forward<Args>(args)...);
        }
    }
};

} // namespace atom::utils
