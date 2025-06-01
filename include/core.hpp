#pragma once
#include <any>
#include <cstdint>
#include <tuple>
#include <type_traits>

#define UTILS ::atom::utils::

namespace atom {

namespace utils {

template <typename Ty>
concept pointer = std::is_pointer_v<Ty>;

using id_t      = uint32_t;
using long_id_t = uint64_t;

#ifndef LONG_ID_TYPE
using default_id_t = id_t;
#else
using default_id_t = long_id_t;
#endif

template <typename, typename>
class compressed_pair;

template <typename, typename>
class reversed_compressed_pair;

template <typename, typename>
struct reversed_pair;

template <typename Rng, typename Closure>
struct pipline_result;

template <typename Fn, typename... Args>
class closure;

template <typename Fn, typename... Args>
constexpr static auto make_closure(Args&&... args) -> closure<Fn, std::decay_t<Args>...>;

template <auto>
struct spreader;

template <typename>
struct type_spreader;

template <typename = void>
class type;

inline namespace _any {

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

struct any_operation_copy_construct {
    void (*value)(void* ptr, const void* other) = nullptr;
};

struct any_operation_move_construct {
    void (*value)(void* ptr, void* other) = nullptr;
};

struct any_operation_copy_assign {
    void (*value)(void* ptr, const void* other) = nullptr;
};

struct any_operation_move_assign {
    void (*value)(void* ptr, void* other) = nullptr;
};

constexpr size_t k_default_any_storege_size = 16;

template <
    size_t Size = k_default_any_storege_size, size_t Align = k_default_any_storege_size,
    typename Ops = std::tuple<
        any_operation_copy_construct, any_operation_move_construct, any_operation_copy_assign,
        any_operation_move_assign>>
class basic_any;

/*! @cond TURN_OFF_DOXYGEN */

template <typename Ty>
struct _not_any : std::true_type {};
template <size_t Size, size_t Align, typename Ops>
struct _not_any<basic_any<Size, Align, Ops>> : std::false_type {};
template <typename Ty>
constexpr auto _not_any_v = _not_any<Ty>::value;

/*! @endcond */

/**
 * @brief Any is a type-erased container that can hold any type of object.
 * @warning This class is designed for high-performance rather than safe access.
 * @tparam Size Size of the builtin storage.
 * @tparam Align Alignment of the builtin storage.
 */
template <size_t Size, size_t Align, typename Ops>
class basic_any {
    bool is_builtin_;

    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
    // NOLINTBEGIN(modernize-avoid-c-arrays)

    /// @brief Storage for small objects.
    /// @details The storage is the first part of the any object, which could be realign easily.
    alignas(Align) std::byte storage_[Size];

    // NOLINTEND(modernize-avoid-c-arrays)
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays)

    /// @brief Pointer to the object stored in the any.
    void* ptr_;

    struct _any_operation_destroy {
        void (*value)(void* ptr) = nullptr;
    };

    using ops_t = decltype(std::tuple_cat(Ops{}, std::tuple<_any_operation_destroy>{}));

    /// @brief Operations for the any object.
    /// @details The operations are used to perform operations on the object stored in the any.
    ops_t operations_;

    static_assert(Size, "Size must be greater than 0");

    template <typename Ty>
    constexpr static auto builtin_storable = sizeof(Ty) <= Size && alignof(Ty) <= Align;

    template <typename Ty>
    consteval static inline auto _operations() noexcept {
        ops_t ops;
        if constexpr (has_type_v<any_operation_copy_construct, ops_t>) {
            std::get<tuple_first_v<any_operation_copy_construct, ops_t>>(ops).value =
                [](void* lhs, const void* rhs) noexcept(std::is_nothrow_copy_constructible_v<Ty>) {
                    new (lhs) Ty(*static_cast<const Ty*>(rhs));
                };
        }
        if constexpr (has_type_v<any_operation_move_construct, ops_t>) {
            std::get<tuple_first_v<any_operation_move_construct, ops_t>>(ops).value =
                [](void* lhs, void* rhs) noexcept(std::is_nothrow_move_constructible_v<Ty>) {
                    new (lhs) Ty(std::move(*static_cast<Ty*>(rhs)));
                };
        }
        if constexpr (has_type_v<any_operation_copy_assign, ops_t>) {
            std::get<tuple_first_v<any_operation_copy_assign, ops_t>>(ops).value =
                [](void* lhs, const void* rhs) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
                    *static_cast<Ty*>(lhs) = *static_cast<const Ty*>(rhs);
                };
        }
        if constexpr (has_type_v<any_operation_move_assign, ops_t>) {
            std::get<tuple_first_v<any_operation_move_assign, ops_t>>(ops).value =
                [](void* lhs, void* rhs) noexcept(std::is_nothrow_move_assignable_v<Ty>) {
                    *static_cast<Ty*>(lhs) = std::move(*static_cast<Ty*>(rhs));
                };
        }
        std::get<tuple_first_v<_any_operation_destroy, ops_t>>(ops).value =
            [](void* ptr) noexcept(std::is_nothrow_destructible_v<Ty>) {
                static_cast<Ty*>(ptr)->~Ty();
            };
        return ops;
    }

    template <typename Ty>
    [[nodiscard]] constexpr void* _init() {
        if constexpr (builtin_storable<Ty>) {
            return static_cast<std::byte*>(storage_);
        }
        else {
            return operator new(sizeof(Ty), std::align_val_t(Align));
        }
    }

public:
    basic_any() : is_builtin_(), ptr_(nullptr), storage_() {}

    template <typename Ty>
    requires _not_any_v<Ty>
    basic_any(Ty&& value)
        : is_builtin_(builtin_storable<Ty>), ptr_(nullptr), storage_(),
          operations_(_operations<std::remove_cvref_t<Ty>>()) {
        auto* ptr = _init<Ty>();
        new (ptr) Ty(std::forward<Ty>(value));
        ptr_ = std::launder(static_cast<Ty*>(ptr));
    }

    // TODO:
    basic_any(const basic_any& that)
    requires has_type_v<any_operation_copy_construct, ops_t>
        : is_builtin_(that.is_builtin_), ptr_(), storage_() {}

    // TODO:
    basic_any(basic_any&& that) noexcept
    requires has_type_v<any_operation_move_construct, ops_t>
        : is_builtin_(that.is_builtin_), ptr_(), storage_() {}

    // FIX:
    basic_any& operator=(const basic_any& that)
    requires has_type_v<any_operation_copy_assign, ops_t>
    {
        if (this != &that) {
            if (ptr_) {
                std::get<tuple_first_v<_any_operation_destroy, ops_t>>(operations_).value(ptr_);
                if (!is_builtin_) {
                    delete static_cast<std::byte*>(ptr_);
                }
            }
            is_builtin_ = that.is_builtin_;
            ptr_        = that.ptr_;
            operations_ = that.operations_;
            if (ptr_) {
                std::get<tuple_first_v<any_operation_copy_construct, ops_t>>(operations_)
                    .value(ptr_, that.ptr_);
            }
        }
        return *this;
    }

    basic_any& operator=(basic_any&& that) noexcept
    requires has_type_v<any_operation_move_assign, ops_t>
    {
        if (this != &that) {
            if (ptr_) {
                std::get<tuple_first_v<_any_operation_destroy, ops_t>>(operations_).value(ptr_);
                if (!is_builtin_) {
                    delete static_cast<std::byte*>(ptr_);
                }
            }
            is_builtin_ = that.is_builtin_;
            ptr_        = that.ptr_;
            operations_ = that.operations_;
            if (ptr_) {
                std::get<tuple_first_v<any_operation_move_construct, ops_t>>(operations_)
                    .value(ptr_, that.ptr_);
            }
        }
        return *this;
    }

    // NOLINTBEGIN(cppcoreguidelines-missing-std-forward)

    template <typename Ty>
    requires _not_any_v<Ty>
    basic_any& operator=(Ty&& value) noexcept {
        if (ptr_) {
            if constexpr (has_type_v<any_operation_copy_assign, ops_t>) {
                std::get<tuple_first_v<any_operation_copy_assign, ops_t>>(operations_)
                    .value(ptr_, &value);
            }
        }
        else {
            auto* ptr = _init<Ty>();
            if constexpr (has_type_v<any_operation_copy_construct, ops_t>) {
                std::get<tuple_first_v<any_operation_copy_construct, ops_t>>(operations_)
                    .value(ptr, &value);
                ptr_ = std::launder(static_cast<Ty*>(ptr));
            }
            else if constexpr (has_type_v<any_operation_copy_assign, ops_t>) {}
        }
    }

    // NOLINTEND(cppcoreguidelines-missing-std-forward)

    ~basic_any() {
        if (ptr_) {
            std::get<tuple_first_v<_any_operation_destroy, ops_t>>(operations_).value(ptr_);
            if (!is_builtin_) {
                operator delete(ptr_);
            }
            ptr_ = nullptr;
        }
    }

    [[nodiscard]] constexpr operator bool() const noexcept { return ptr_; }

    template <pointer Ty>
    constexpr Ty _cast() noexcept {
        return static_cast<Ty>(ptr_);
    }

    template <pointer Ty>
    constexpr Ty _cast() const noexcept {
        return static_cast<Ty>(ptr_);
    }

    template <typename Ty>
    constexpr Ty _cast() noexcept {
        return *static_cast<std::remove_cvref_t<Ty>*>(ptr_);
    }

    template <typename Ty>
    constexpr const Ty _cast() const noexcept {
        return *static_cast<const std::remove_cvref_t<Ty>*>(ptr_);
    }
};
using any = basic_any<>;

template <pointer Ty, size_t Size, size_t Align, typename Ops>
constexpr inline Ty any_cast(basic_any<Size, Align, Ops>& any) noexcept {
    // you should ensure that the type is correct before calling this function.
    // if you meet an error, please ask yourself.
    return any.template _cast<std::remove_cvref_t<Ty>>();
}

template <pointer Ty, size_t Size, size_t Align, typename Ops>
constexpr inline Ty any_cast(const basic_any<Size, Align, Ops>& any) noexcept {
    // you should ensure that the type is correct before calling this function.
    // if you meet an error, please ask yourself.
    return any.template _cast<const std::remove_cvref_t<Ty>>();
}

template <typename Ty, size_t Size, size_t Align, typename Ops>
constexpr inline Ty& any_cast(basic_any<Size, Align, Ops>& any) noexcept {
    // you should ensure that the type is correct before calling this function.
    // if you meet an error, please ask yourself.
    return any.template _cast<Ty&>();
}

template <typename Ty, size_t Size, size_t Align, typename Ops>
constexpr inline const Ty& any_cast(const basic_any<Size, Align, Ops>& any) noexcept {
    // you should ensure that the type is correct before calling this function.
    // if you meet an error, please ask yourself.
    return any.template _cast<const Ty&>();
}

} // namespace _any

} // namespace utils

using default_id_t = utils::default_id_t;

} // namespace atom
