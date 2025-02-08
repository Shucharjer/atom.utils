#pragma once
#include <atomic>
#include <new>
#include <type_traits>
#include <utility>
#include "concepts/allocator.hpp"
#include "core.hpp"
#include "core/pair.hpp"
#include "memory.hpp"
#include "memory/allocator.hpp"

namespace atom::utils {

template <typename Ty>
struct default_destroyer;

namespace internal {

template <typename Destroyer, typename Ty>
inline constexpr void destroy(
    Ty* const ptr, const Destroyer& destroyer
) noexcept(std::is_nothrow_destructible_v<Ty>) {
    static_assert(std::is_invocable_v<Destroyer, void* const>);
    destroyer(static_cast<void* const>(ptr));
}

template <typename Begin, typename End>
constexpr void destroy_range(
    Begin begin, End end
) noexcept(std::is_nothrow_destructible_v<std::iter_value_t<Begin>>) {
    using value_type = std::iter_value_t<Begin>;
    if constexpr (!std::is_trivially_destructible_v<value_type>) {
        for (; begin != end; ++begin) {
            (*begin)->~value_type();
        }
    }
}

template <typename Ty>
constexpr void destroy(Ty* const ptr) noexcept(std::is_nothrow_destructible_v<Ty>) {
    if constexpr (std::is_array_v<Ty>) {
        destroy_range(std::begin(*ptr), std::end(*ptr));
    }
    else if constexpr (!std::is_trivially_destructible_v<Ty>) {
        ptr->~Ty();
    }
}

} // namespace internal

class basic_storage {
public:
    constexpr basic_storage()                                = default;
    constexpr basic_storage(const basic_storage&)            = default;
    constexpr basic_storage(basic_storage&&)                 = default;
    constexpr basic_storage& operator=(const basic_storage&) = default;
    constexpr basic_storage& operator=(basic_storage&&)      = default;
    constexpr virtual ~basic_storage()                       = default;

    constexpr virtual explicit operator bool() const noexcept { return false; }

    constexpr virtual auto raw() noexcept -> void* { return nullptr; }
    [[nodiscard]] constexpr virtual auto raw() const noexcept -> const void* { return nullptr; }
};

/**
 * @brief Lazy storage.
 * Initialize on Get, Copy on Write.
 */
template <
    typename Ty,
    ::atom::utils::concepts::rebindable_allocator Allocator = ::atom::utils::standard_allocator<Ty>>
class unique_storage;

/**
 * @brief Shared lazy storage.
 * Initialize on Get, Copy on Write.
 */
template <
    typename Ty,
    ::atom::utils::concepts::rebindable_allocator Allocator = ::atom::utils::standard_allocator<Ty>>
class shared_storage;

template <typename Ty>
struct default_destroyer {
    using value_type = Ty;

    template <typename T>
    using rebind_t = default_destroyer<T>;

    inline constexpr void operator()(Ty* const ptr) const noexcept { internal::destroy(ptr); }
};

struct with_allocator_t {};
struct with_destroyer_t {};

constexpr inline with_allocator_t with_allocator;
constexpr inline with_destroyer_t with_destroyer;

struct construct_at_once_t {};

constexpr inline construct_at_once_t construct_at_once;

namespace internal {

template <typename Ty, typename Destroyer>
constexpr auto wrap_destroyer(Destroyer destroyer) -> void (*)(void*) {
    static_assert(std::is_invocable_v<Destroyer, Ty*>);
    if constexpr (std::is_function_v<Destroyer>) {
        return [&destroyer](void* ptr) { destroyer(static_cast<Ty*>(ptr)); };
    }
    else if constexpr (std::is_class_v<Destroyer>) {
        return [](void* ptr) { Destroyer{}(static_cast<Ty*>(ptr)); };
    }
    else {
        static_assert(false);
        return nullptr;
    }
}

} // namespace internal

template <typename Ty, ::atom::utils::concepts::rebindable_allocator Allocator>
class unique_storage : public basic_storage {
    using alty_traits = std::allocator_traits<Allocator>;

public:
    using count_type = uint32_t;

    using value_type      = Ty;
    using allocator_type  = Allocator;
    using pointer         = typename alty_traits::pointer;
    using const_pointer   = typename alty_traits::const_pointer;
    using reference       = Ty&;
    using const_reference = const Ty&;

    static_assert(sizeof(Ty), "Can't suit for incompleted type");
    static_assert(!std::is_const_v<Ty>);

    constexpr explicit unique_storage(
        const Allocator& allocator = Allocator()
    ) noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
        : pair_(internal::wrap_destroyer<Ty>(default_destroyer<Ty>{}), allocator), val_(nullptr) {}

    constexpr explicit unique_storage(
        const Ty* ptr, const Allocator& allocator = Allocator()
    ) noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
        : pair_(internal::wrap_destroyer<Ty>(default_destroyer<Ty>{}), allocator), val_(ptr) {}

    template <typename Destroyer>
    constexpr explicit unique_storage(const Ty* ptr, with_destroyer_t, Destroyer destroyer)
        : pair_(internal::wrap_destroyer<Ty>(destroyer), Allocator()), val_(ptr) {}

    constexpr explicit unique_storage(construct_at_once_t, const Allocator& allocator = Allocator{})
        : pair_(internal::wrap_destroyer<Ty>(default_destroyer<Ty>{}), allocator), val_(nullptr) {
        Ty* ptr = nullptr;
        try {
            ptr = pair_.second().allocate(1);
            ::new (ptr) Ty();
            val_ = std::launder(ptr);
        }
        catch (...) {
            if (ptr) {
                pair_.second().deallocate(ptr, 1);
            }
            throw;
        }
    }

    template <typename... Args>
    explicit unique_storage(Args&&... args, const Allocator& allocator = Allocator{})
        : pair_(internal::wrap_destroyer<Ty>(default_destroyer<Ty>{}), allocator), val_(nullptr) {
        Ty* ptr = nullptr;
        try {
            ptr = pair_.second().allocate(1);
            ::new (ptr) Ty(std::forward<Args>(args)...);
            val_ = std::launder(ptr);
        }
        catch (...) {
            if (ptr) {
                pair_.second().deallocate(ptr, 1);
            }
            throw;
        }
    }

    unique_storage(const unique_storage& that) = delete;

    constexpr unique_storage(unique_storage&& that) noexcept
        : pair_(std::move(that.pair_)), val_(std::exchange(that.val_, nullptr)) {}

    unique_storage& operator=(const unique_storage& that) = delete;

    constexpr unique_storage& operator=(unique_storage&& that) noexcept {
        if (this != &that) {
            pair_ = std::move(that.pair_);
            val_  = std::exchange(that.val_, nullptr);
        }

        return *this;
    }

    constexpr ~unique_storage() noexcept(std::is_nothrow_destructible_v<Ty>) override { release(); }

    auto raw() noexcept -> void* override { return static_cast<void*>(val_); }
    [[nodiscard]] auto raw() const noexcept -> const void* override {
        return static_cast<void*>(val_);
    }

    constexpr explicit operator bool() const noexcept override { return val_; }

    constexpr auto get() noexcept -> Ty* { return val_; }
    [[nodiscard]] constexpr auto get() const noexcept -> const Ty* { return val_; }

    template <typename Val>
    unique_storage& operator=(Val&& val) {
        if (val_) {
            *val_ = std::forward<Val>(val);
        }
        else {
            auto* ptr = pair_.second().allocate();
            ::new (ptr) Ty(std::forward<Val>(val));
            val_ = std::launder(ptr);
        }

        return *this;
    }

    constexpr auto operator->() noexcept -> Ty* { return val_; }
    [[nodiscard]] constexpr auto operator->() const noexcept -> const Ty* { return val_; }

    constexpr void reset() {
        if (val_) [[likely]] {
            pair_.first()(static_cast<void*>(val_));
            pair_.second().deallocate(val_, 1);
            val_ = nullptr;
        }
    }

    constexpr void release() {
        if (val_) [[likely]] {
            pair_.first()(static_cast<void*>(val_));
            pair_.second().deallocate(val_, 1);
            val_ = nullptr;
        }
    }

private:
    compressed_pair<void (*)(void*), allocator_type> pair_;
    Ty* val_;
};

template <typename Ty, ::atom::utils::concepts::rebindable_allocator Allocator>
class shared_storage final : public basic_storage {
    using alty_traits = std::allocator_traits<Allocator>;

public:
    using meta_count_type = uint32_t;
    using count_type      = std::atomic<meta_count_type>;

    using value_type           = Ty;
    using allocator_type       = Allocator;
    using count_allocator_type = typename Allocator::template rebind_t<count_type>;
    using pointer              = typename alty_traits::pointer;
    using const_pointer        = typename alty_traits::const_pointer;
    using reference            = Ty&;
    using const_reference      = const Ty&;

    using destroyer_type = void (*)(void*);

    // default constructor
    constexpr explicit shared_storage() noexcept
        : pair_(nullptr, allocator_type{}),
          control_pair_(nullptr, internal::wrap_destroyer<Ty>(default_destroyer<Ty>{})) {}

    // with ptr
    template <typename T>
    constexpr explicit shared_storage(T* ptr)
        : pair_(ptr, allocator_type{}),
          control_pair_(nullptr, internal::wrap_destroyer<Ty>(default_destroyer<Ty>{})) {
        auto count_allocator = count_allocator_type{};
        count_type* count    = nullptr;
        try {
            count = count_allocator.allocate();
            ::new (count) count_type(1);
            control_pair_.first() = std::launder(count);
        }
        catch (...) {
            if (count) {
                count->~count_type();
            }
            throw;
        }
    }

    // only allocator
    constexpr explicit shared_storage(const Allocator& allocator) noexcept
        : pair_(nullptr, allocator),
          control_pair_(nullptr, internal::wrap_destroyer<Ty>(default_destroyer<Ty>{})) {}

    // destroyer
    template <typename Destroyer = default_destroyer<Ty>>
    constexpr explicit shared_storage(with_destroyer_t, Destroyer destroyer) noexcept
        : pair_(nullptr, allocator_type{}),
          control_pair_(nullptr, internal::wrap_destroyer<Ty>(destroyer)) {}

    // with ptr & allocator
    template <typename T>
    constexpr explicit shared_storage(T* ptr, const Allocator& allocator)
        : pair_(ptr, allocator),
          control_pair_(nullptr, internal::wrap_destroyer<Ty>(default_destroyer<Ty>{})) {
        auto count_allocator = count_allocator_type{ allocator };
        count_type* count    = nullptr;
        try {
            count = count_allocator.allocate();
            ::new (count) count_type(1);
            control_pair_.first() = std::launder(count);
        }
        catch (...) {
            if (count) {
                count->~count_type();
                count_allocator.deallocate(count);
            }
            throw;
        }
    }

    // ptr & destroyer
    template <typename Destroyer = default_destroyer<Ty>>
    constexpr explicit shared_storage(const Ty* ptr, with_destroyer_t, Destroyer destroyer)
        : pair_(ptr, allocator_type{}),
          control_pair_(
              count_allocator_type().allocate(), internal::wrap_destroyer<Ty>(destroyer)
          ) {
        control_pair_.first()->store(1, std::memory_order_release);
    }

    // allocator & destroyer
    template <typename Destroyer = default_destroyer<Ty>>
    constexpr explicit shared_storage(const Allocator& allocator, Destroyer destroyer) noexcept
        : pair_(nullptr, allocator),
          control_pair_(nullptr, internal::wrap_destroyer<Ty>(destroyer)) {}

    // ptr, allocator & destroyer
    template <typename T, typename Destroyer = default_destroyer<Ty>>
    constexpr explicit shared_storage(T* ptr, const Allocator& allocator, Destroyer destroyer)
        : pair_(ptr, allocator),
          control_pair_(count_allocator_type(allocator), internal::wrap_destroyer<Ty>(destroyer)) {}

    // arguments
    template <typename... Args>
    requires std::is_constructible_v<Ty, Args...>
    explicit shared_storage(Args&&... args)
        : pair_(nullptr, allocator_type{}),
          control_pair_(nullptr, internal::wrap_destroyer<Ty>(default_destroyer<Ty>{})) {
        Ty* ptr = nullptr;
        try {
            ptr = pair_.second().allocate();
            ::new (ptr) Ty(std::forward<Args>(args)...);
            pair_.first() = std::launder(ptr);

            count_type* count    = nullptr;
            auto count_allocator = count_allocator_type{ pair_.second() }.allocate();
            try {
                count = count_allocator.allocate();
                ::new (count) count_type(1);
                control_pair_.first() = std::launder(count);
            }
            catch (...) {
                if (count) {
                    count_allocator.deallocate(count);
                }
                throw;
            }
        }
        catch (...) {
            if (ptr) {
                ptr->~Ty();
                pair_.second().deallocate(ptr);
                if (pair_.first()) {
                    pair_.first() = nullptr;
                }
            }
            throw;
        }
    }

    template <typename... Args>
    requires std::is_constructible_v<Ty, Args...>
    constexpr explicit shared_storage(Args&&... args, const Allocator& allocator)
        : pair_(nullptr, allocator),
          control_pair_(nullptr, internal::wrap_destroyer<Ty>(default_destroyer<Ty>())) {
        Ty* ptr = nullptr;
        try {
            ptr = pair_.second().allocate();
            ::new (ptr) Ty(std::forward<Args>(args)...);
            pair_.first() = std::launder(ptr);

            count_type* count    = nullptr;
            auto count_allocator = count_allocator_type{ pair_.second() }.allocate();
            try {
                count = count_allocator.allocate();
                ::new (count) count_type(1);
                control_pair_.first() = std::launder(count);
            }
            catch (...) {
                if (count) {
                    count_allocator.deallocate(count);
                }
                throw;
            }
        }
        catch (...) {
            if (ptr) {
                ptr->~Ty();
                pair_.second().deallocate(ptr);
                if (pair_.first()) {
                    pair_.first() = nullptr;
                }
            }
            throw;
        }
    }

    template <typename... Args, typename Destroyer = default_destroyer<Ty>>
    requires std::is_constructible_v<Ty, Args...>
    constexpr explicit shared_storage(Args&&... args, with_destroyer_t, Destroyer destroyer)
        : pair_(nullptr, allocator_type{}),
          control_pair_(nullptr, internal::wrap_destroyer<Ty>(destroyer)) {
        Ty* ptr = nullptr;
        try {
            ptr = pair_.second().allocate();
            ::new (ptr) Ty(std::forward<Args>(args)...);
            pair_.first() = std::launder(ptr);

            count_type* count    = nullptr;
            auto count_allocator = count_allocator_type{ pair_.second() };
            try {
                count = count_allocator.allocate();
                ::new (count) count_type(1);
                // noexcept
                control_pair_.first() = std::launder(count);
            }
            catch (...) {
                if (count) {
                    count_allocator.deallocate(count);
                    count = nullptr;
                }
                throw;
            }
        }
        catch (...) {
            if (ptr) {
                pair_.second().deallocate(ptr);
                if (pair_.first()) {
                    pair_.first() = nullptr;
                }
            }
            throw;
        }
    }

    shared_storage(const shared_storage& that) noexcept
        : pair_(that.pair_), control_pair_(that.control_pair_) {
        inc();
    }

    shared_storage(shared_storage&& that) noexcept
        : pair_(std::move(that.pair_)), control_pair_(std::move(that.control_pair_)) {}

    template <typename T>
    explicit shared_storage(const shared_storage<T, Allocator>& that) noexcept
        : pair_(that.pair_), control_pair_(that.control_pair_) {
        inc();
    }

    template <typename T>
    explicit shared_storage(shared_storage<T, Allocator>&& that) noexcept
        : pair_(std::move(that.pair_)), control_pair_(std::move(that.control_pair_)) {}

    shared_storage& operator=(const shared_storage& that) noexcept {
        if (this != &that) {
            dec();
            pair_         = that.pair_;
            control_pair_ = that.control_pair_;
            inc();
        }

        return *this;
    }

    shared_storage& operator=(shared_storage&& that) noexcept {
        if (this != &that) {
            dec();
            pair_         = std::move(that.pair_);
            control_pair_ = std::move(that.control_pair_);
        }

        return *this;
    }

    template <typename T>
    shared_storage& operator=(const shared_storage<T, Allocator>& that) noexcept {
        dec();
        pair_         = that.pair_;
        control_pair_ = that.control_pair_;
        inc();

        return *this;
    }

    template <typename T>
    shared_storage& operator=(shared_storage<T, Allocator>&& that) noexcept {
        dec();
        pair_         = std::move(that.pair_);
        control_pair_ = std::move(that.control_pair_);

        return *this;
    }

    ~shared_storage() override { dec(); }

    auto raw() noexcept -> void* override { return static_cast<void*>(pair_.first()); }
    [[nodiscard]] auto raw() const noexcept -> const void* override {
        return static_cast<const void*>(pair_.first());
    }

    template <typename Val>
    shared_storage& operator=(Val&& val) {
        static_assert(std::is_constructible_v<Ty, Val> || std::is_default_constructible_v<Ty>);

        auto& val_   = pair_.first();
        auto& count_ = control_pair_.first();

        if (count_ && count_->load() == 1) {
            *val_ = std::forward<Val>(val);
        }
        else {
            val_ = pair_.second().allocate();
            if constexpr (std::is_constructible_v<Ty, Val>) {
                ::new (val_) Ty(std::forward<Val>(val));
                val_ = std::launder(val_);
            }
            else if constexpr (std::is_default_constructible_v<Ty>) {
                ::new (val_) Ty();
                val_  = std::launder(val_);
                *val_ = std::forward<Val>(val);
            }
            count_ = count_allocator_type{ pair_.second() }.allocate();
            ::new (count_) count_type(1);
            count_ = std::launder(count_);
        }

        return *this;
    }

    constexpr explicit operator bool() const noexcept override { return control_pair_.first(); }

    auto get() noexcept -> Ty* { return pair_.first(); }
    auto get() const noexcept -> const Ty* { return pair_.first(); }

    template <typename T = void>
    void reset(T* ptr = nullptr) {
        dec();
        pair_.first()         = ptr;
        control_pair_.first() = ptr ? ::new count_type(1) : nullptr;
    }

    void release() {
        dec();
        pair_.first()         = nullptr;
        control_pair_.first() = nullptr;
    }

    [[nodiscard]] auto count() const noexcept {
        return control_pair_.first()->load(std::memory_order_relaxed);
    }

private:
    inline constexpr void inc() {
        if (control_pair_.first()) {
            control_pair_.first()->fetch_add(1, std::memory_order_relaxed);
        }
    }

    inline constexpr void dec() {
        if (control_pair_.first()) {
            meta_count_type expected = 1;
            if (control_pair_.first()->load(std::memory_order_acquire) == 1 &&
                control_pair_.first()->compare_exchange_strong(
                    expected, 0, std::memory_order_acq_rel
                )) {
                control_pair_.second()(static_cast<void*>(pair_.first()));
                pair_.second().deallocate(pair_.first());
                pair_.first() = nullptr;
                control_pair_.first()->~count_type();
                count_allocator_type{ pair_.second() }.deallocate(control_pair_.first());
                control_pair_.first() = nullptr;
            }
            else {
                control_pair_.first()->fetch_sub(1, std::memory_order_release);
            }
        }
    }

    compressed_pair<Ty*, allocator_type> pair_;
    compressed_pair<count_type*, destroyer_type> control_pair_;
};

} // namespace atom::utils
