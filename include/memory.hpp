#pragma once
#include <memory>
#include <type_traits>
#include "core.hpp"

namespace atom::utils {

struct allocator_object {
    template <typename Base>
    struct interface : Base {
        void* allocate() { return this->template invoke<0>(); }
        void deallocate(void* ptr) { this->template invoke<1>(ptr); }
    };

    template <typename Impl>
    using impl = value_list<&Impl::allocate, &Impl::deallocate>;
};

template <typename Alloc>
concept _count_allocator = requires(Alloc& alloc) {
    { alloc.allocate(std::declval<size_t>()) } -> std::convertible_to<void*>;
};

template <typename Alloc>
concept _allocator = requires(Alloc& alloc) {
    { alloc.allocate() } -> std::convertible_to<void*>;
};

template <typename Ty, _count_allocator Alloc>
struct allocator : private Alloc {
    constexpr allocator() noexcept(std::is_nothrow_default_constructible_v<Alloc>) = default;

    template <typename Al>
    constexpr allocator(const Al& alloc) noexcept(std::is_nothrow_constructible_v<Alloc, Al>)
        : Alloc(alloc) {}

    [[nodiscard]] void* allocate() { return static_cast<Alloc*>(this)->allocate(1); }

    void deallocate(void* ptr) { static_cast<Alloc*>(this)->deallocate(static_cast<Ty*>(ptr), 1); }
};

/**
 * @class common_allocator
 * @brief Static polymorphic allocator object.
 * @note This class is slightly faster when high frequency allocating and deallocating memory than
 * `common_tiny_allocator`.
 * @details When allocating/deallocating, it will use ptr_ directly, which is faster than
 * `common_tiny_allocator`, so when the address of `ptr_` is on the cache, it will be faster.
 */
class common_allocator {
    using vtable_t = vtable<allocator_object>;
    vtable_t vtable_;
    void* ptr_;
    void (*destroy_)(void* ptr);

public:
    using poly_tag = void;

    void* allocate() { return std::get<0>(vtable_)(ptr_); }
    void deallocate(void* ptr) { std::get<1>(vtable_)(ptr_, ptr); }

    common_allocator(vtable_t vtable, void* allocator, void (*destroy)(void*)) noexcept
        : vtable_(std::move(vtable)), ptr_(allocator), destroy_(destroy) {}

    common_allocator(const common_allocator&)            = delete;
    common_allocator(common_allocator&&)                 = delete;
    common_allocator& operator=(const common_allocator&) = delete;
    common_allocator& operator=(common_allocator&&)      = delete;

    ~common_allocator() { destroy_(ptr_); }
};

/**
 * @brief Creates a common allocator object.
 * Now you could use a container such as `std::vector` store all kinds of allocators,
 * which is useful for ECS (Entity Component System) or other systems that requires highly flexible
 * memory management.
 */
template <typename Ty, _count_allocator Alloc>
requires _poly_impl<allocator<Ty, Alloc>, allocator_object>
CONSTEXPR23 inline common_allocator make_common_allocator() {
    using allocator = allocator<Ty, Alloc>;
    return common_allocator{ make_vtable<allocator_object, allocator>(), new allocator(),
                             [](void* ptr) { delete static_cast<allocator*>(ptr); } };
}

/**
 * @brief Creates a common allocator object.
 * Now you could use a container such as `std::vector` store all kinds of allocators,
 * which is useful for ECS (Entity Component System) or other systems that requires highly flexible
 * memory management.
 */
template <typename Ty, _allocator Alloc>
CONSTEXPR23 inline common_allocator make_common_allocator() {
    return common_allocator{ make_vtable<allocator_object, Alloc>(), new Alloc(),
                             [](void* ptr) { delete static_cast<Alloc*>(ptr); } };
}

/**
 * @brief Creates a common allocator object.
 * Now you could use a container such as `std::vector` store all kinds of allocators,
 * which is useful for ECS (Entity Component System) or other systems that requires highly flexible
 * memory management.
 */
template <typename Ty, _count_allocator Alloc, typename Al>
requires _poly_impl<allocator<Ty, Alloc>, allocator_object>
CONSTEXPR23 inline common_allocator make_common_allocator(const Al& alloc) {
    using allocator = allocator<Ty, Alloc>;
    return common_allocator{ make_vtable<allocator_object, allocator>(), new allocator(alloc),
                             [](void* ptr) { delete static_cast<allocator*>(ptr); } };
}

/**
 * @brief Creates a common allocator object.
 * Now you could use a container such as `std::vector` store all kinds of allocators,
 * which is useful for ECS (Entity Component System) or other systems that requires highly flexible
 * memory management.
 */
template <typename Ty, template <typename> typename Alloc>
requires _poly_impl<Alloc<Ty>, allocator_object> && _allocator<Alloc<Ty>>
CONSTEXPR23 inline common_allocator make_common_allocator() {
    return common_allocator{ make_vtable<allocator_object, Alloc<Ty>>(), new Alloc(),
                             [](void* ptr) { delete static_cast<Alloc<Ty>*>(ptr); } };
}

/**
 * @brief Creates a common allocator object.
 * Now you could use a container such as `std::vector` store all kinds of allocators,
 * which is useful for ECS (Entity Component System) or other systems that requires highly flexible
 * memory management.
 */
template <typename Ty, template <typename> typename Alloc = std::allocator>
requires _poly_impl<allocator<Ty, Alloc<Ty>>, allocator_object> && _count_allocator<Alloc<Ty>>
CONSTEXPR23 inline common_allocator make_common_allocator() {
    using allocator = allocator<Ty, Alloc<Ty>>;
    return common_allocator{ make_vtable<allocator_object, allocator>(), new allocator(),
                             [](void* ptr) { delete static_cast<allocator*>(ptr); } };
}

/**
 * @brief Creates a common allocator object.
 * Now you could use a container such as `std::vector` store all kinds of allocators,
 * which is useful for ECS (Entity Component System) or other systems that requires highly flexible
 * memory management.
 */
template <typename Ty, template <typename> typename Alloc = std::allocator, typename Al>
requires _poly_impl<allocator<Ty, Alloc<Ty>>, allocator_object> && _count_allocator<Alloc<Ty>>
CONSTEXPR23 inline common_allocator make_common_allocator(const Al& alloc) {
    using allocator = allocator<Ty, Alloc<Ty>>;
    return common_allocator{ make_vtable<allocator_object, allocator>(), new allocator(alloc),
                             [](void* ptr) { delete static_cast<allocator*>(ptr); } };
}

constexpr inline size_t _tiny_allocator_size = 8;

/**
 * @class common_tiny_allocator
 * @brief Static polymorphic allocator object for small allocators.
 * @note This class is designed for small allocators that can fit in a small buffer.
 * It is useful for not so frequently allocating and deallocating memory. in other words, it's
 * faster when the allocator is not always in cache.
 * @details When allocating/deallocating, it will calculate the offset of its member `bytes_`,
 * causing a performance loss. So, ballencing the calling frequency.
 */
class common_tiny_allocator {
    using vtable_t = vtable<allocator_object>;
    vtable_t vtable_;

    void (*destroy_)(void* ptr);

    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
    // NOLINTBEGIN(modernize-avoid-c-arrays)

    std::byte bytes_[_tiny_allocator_size]{};

    // NOLINTEND(modernize-avoid-c-arrays)
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays)

public:
    using poly_tag = void;

    constexpr void* allocate() { return std::get<0>(vtable_)(static_cast<void*>(bytes_)); }
    constexpr void deallocate(void* ptr) { std::get<1>(vtable_)(static_cast<void*>(bytes_), ptr); }

    template <_not<common_allocator, common_tiny_allocator> Ty>
    constexpr common_tiny_allocator(
        vtable_t vtable, const Ty& alloc, void (*destroy)(void*)) noexcept
        : vtable_(std::move(vtable)), destroy_(destroy) {
        ::new (static_cast<void*>(bytes_)) Ty(alloc);
    }

    common_tiny_allocator(const common_tiny_allocator&)            = delete;
    common_tiny_allocator(common_tiny_allocator&&)                 = delete;
    common_tiny_allocator& operator=(const common_tiny_allocator&) = delete;
    common_tiny_allocator& operator=(common_tiny_allocator&&)      = delete;

    constexpr ~common_tiny_allocator() { destroy_(static_cast<void*>(bytes_)); }
};

template <typename Ty, _count_allocator Alloc>
requires _poly_impl<allocator<Ty, Alloc>, allocator_object>
constexpr inline common_tiny_allocator make_common_tiny_allocator() {
    using allocator = allocator<Ty, Alloc>;
    static_assert(
        sizeof(allocator) <= _tiny_allocator_size, "Allocator too large for tiny allocator");
    return common_tiny_allocator(
        make_vtable<allocator_object, allocator>(), allocator{},
        [](void* ptr) { static_cast<allocator*>(ptr)->~allocator(); });
}

template <typename Ty, _count_allocator Alloc, typename Al>
requires _poly_impl<allocator<Ty, Alloc>, allocator_object>
constexpr inline common_tiny_allocator make_common_tiny_allocator(const Al& alloc) {
    using allocator = allocator<Ty, Alloc>;
    static_assert(
        sizeof(allocator) <= _tiny_allocator_size, "Allocator too large for tiny allocator");
    return common_tiny_allocator(make_vtable<allocator_object, allocator>(), alloc, [](void* ptr) {
        static_cast<allocator*>(ptr)->~allocator();
    });
}

template <typename Ty, template <typename> typename Alloc = std::allocator>
requires _poly_impl<allocator<Ty, Alloc<Ty>>, allocator_object> && _count_allocator<Alloc<Ty>>
constexpr inline common_tiny_allocator make_common_tiny_allocator() {
    using allocator = allocator<Ty, Alloc<Ty>>;
    static_assert(
        sizeof(allocator) <= _tiny_allocator_size, "Allocator too large for tiny allocator");
    return common_tiny_allocator(
        make_vtable<allocator_object, allocator>(), allocator{},
        [](void* ptr) { static_cast<allocator*>(ptr)->~allocator(); });
}

template <typename Ty, template <typename> typename Alloc = std::allocator, typename Al>
requires _poly_impl<allocator<Ty, Alloc<Ty>>, allocator_object> && _count_allocator<Alloc<Ty>>
constexpr inline common_tiny_allocator make_common_tiny_allocator(const Al& alloc) {
    using allocator = allocator<Ty, Alloc<Ty>>;
    static_assert(
        sizeof(allocator) <= _tiny_allocator_size, "Allocator too large for tiny allocator");
    return common_tiny_allocator(make_vtable<allocator_object, allocator>(), alloc, [](void* ptr) {
        static_cast<allocator*>(ptr)->~allocator();
    });
}

template <typename>
struct rebind_allocator;

template <template <typename> typename Allocator, typename Ty>
struct rebind_allocator<Allocator<Ty>> {
    template <typename Other>
    struct to {
        using type = Allocator<Other>;
    };

    template <typename Other>
    using to_t = typename to<Other>::type;
};

template <template <typename...> typename Allocator, typename Ty, typename... Others>
requires((sizeof...(Others) != 0))
struct rebind_allocator<Allocator<Ty, Others...>> {
    template <typename Other>
    struct to {
        using type = Allocator<Other, Others...>;
    };

    template <typename Other>
    using to_t = typename to<Other>::type;
};

template <template <typename, auto...> typename Allocator, typename Ty, auto... Args>
requires((sizeof...(Args) != 0))
struct rebind_allocator<Allocator<Ty, Args...>> {
    template <typename Other>
    struct to {
        using type = Allocator<Other, Args...>;
    };

    template <typename Other>
    using to_t = typename to<Other>::type;
};

} // namespace atom::utils
