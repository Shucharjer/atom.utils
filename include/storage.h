#pragma once
#include <atomic>
#include <functional>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include "allocator.h"
#include "compressed_pair.h"

namespace atom::utils {

template <typename Ty>
struct default_destroyer;

namespace internal {

template <typename Destroyer, typename Ty>
inline constexpr void destroy(
    Ty* const ptr, const Destroyer& destroyer
) noexcept(std::is_nothrow_destructible_v<Ty>) {
    static_assert(std::is_invocable_v<Destroyer, Ty* const>);
    destroyer(ptr);
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

    constexpr virtual auto raw() noexcept -> void* { return nullptr; };
    [[nodiscard]] constexpr virtual auto raw() const noexcept -> const void* { return nullptr; }
};

/**
 * @brief Lazy storage.
 * Initialize on Get, Copy on Write.
 */
template <typename Ty, typename = allocator<Ty, void>, typename = default_destroyer<Ty>>
class unique_storage;

/**
 * @brief Shared lazy storage.
 * Initialize on Get, Copy on Write.
 */
template <typename Ty, typename = allocator<Ty, void>, typename = default_destroyer<Ty>>
class shared_storage;

template <typename Ty>
struct default_destroyer {
    using value_type = Ty;

    template <typename T>
    using rebind_t = default_destroyer<T>;

    inline constexpr void operator()(Ty* const ptr) const noexcept { ptr->~Ty(); }
};

template <typename Ty, typename Allocator, typename Destroyer>
class unique_storage : public basic_storage {
    using alty_traits = std::allocator_traits<Allocator>;

public:
    using count_type = uint32_t;

    using value_type      = Ty;
    using allocator_type  = Allocator;
    using destroyer_type  = Destroyer;
    using pointer         = typename alty_traits::pointer;
    using const_pointer   = typename alty_traits::const_pointer;
    using reference       = Ty&;
    using const_reference = const Ty&;

    static_assert(sizeof(Ty), "Can't suit for incompleted type");
    static_assert(!std::is_const_v<Ty>);

    constexpr explicit unique_storage(
        const Allocator& allocator = Allocator()
    ) noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
        : pair_(nullptr, allocator) {}

    constexpr explicit unique_storage(
        const Ty* ptr, const Allocator& allocator = Allocator()
    ) noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
        : pair_(ptr, allocator) {}

    template <typename... Args>
    requires std::is_constructible_v<Ty, Args...>
    explicit unique_storage(Args&&... args, const Allocator& allocator = Allocator())
        : pair_(allocator.allocate(), allocator) {
        ::new (pair_.first()) Ty(std::forward<Args>(args)...);
    }

    unique_storage(const unique_storage& that) = delete;

    constexpr unique_storage(unique_storage&& that) noexcept : pair_(std::move(that.pair_)) {}

    unique_storage& operator=(const unique_storage& that) = delete;

    constexpr unique_storage& operator=(unique_storage&& that) noexcept {
        if (this != &that) {
            pair_ = std::move(that.pair_);
        }

        return *this;
    }

    constexpr ~unique_storage() noexcept(std::is_nothrow_destructible_v<Ty>) override {
        if (pair_.first()) {
            internal::destroy(pair_.first(), destroyer_);
            pair_.second().deallocate();
        }
    }

    auto raw() noexcept -> void* override { return pair_.first(); }
    [[nodiscard]] auto raw() const noexcept -> const void* override { return pair_.first(); }

    constexpr explicit operator bool() const noexcept override { return pair_.first(); }

    constexpr auto get() noexcept -> Ty* { return pair_.first(); }
    [[nodiscard]] constexpr auto get() const noexcept -> const Ty* { return pair_.first(); }

    template <typename Val>
    unique_storage& operator=(Val&& val) {
        auto& val_ = pair_.first();

        if (val_) {
            *val_ = std::forward<Val>(val);
        }
        else {
            val_ = pair_.second().allocate();
            ::new (val_) Ty(std::forward<Val>(val));
            val_ = std::launder(val_);
        }

        return *this;
    }

    constexpr auto operator->() noexcept -> Ty* { return pair_.first(); }
    [[nodiscard]] constexpr auto operator->() const noexcept -> const Ty* { return pair_.first(); }

    constexpr void reset() {
        auto& val_ = pair_.first();

        if (val_) [[likely]] {
            internal::destroy(val_, destroyer_);
            val_ = nullptr;
        }
    }

    constexpr void release() {
        auto& val_ = pair_.first();

        if (val_) [[likely]] {
            internal::destroy(val_, destroyer_);
            val_ = nullptr;
        }
    }

private:
    compressed_pair<Ty*, allocator_type> pair_;
    Destroyer destroyer_;
};

template <typename Ty, typename Allocator, typename Destroyer>
class shared_storage : public basic_storage {
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

    using destroyer_type = Destroyer;

    constexpr shared_storage() noexcept
        : pair_(nullptr, allocator_type{}), control_pair_(nullptr, Destroyer()) {}

    constexpr explicit shared_storage(const Allocator& allocator) noexcept
        : pair_(nullptr, allocator), control_pair_(nullptr, Destroyer()) {}

    constexpr explicit shared_storage(const Destroyer& destroyer) noexcept
        : pair_(nullptr, allocator_type{}), control_pair_(nullptr, destroyer) {}

    constexpr explicit shared_storage(
        const Allocator& allocator, const Destroyer& destroyer
    ) noexcept
        : pair_(nullptr, allocator) {}

    constexpr explicit shared_storage(const Ty* ptr)
        : pair_(ptr, allocator_type{}),
          control_pair_(count_allocator_type().allocate(), Destroyer()) {}

    constexpr explicit shared_storage(const Ty* ptr, const Allocator& allocator)
        : pair_(ptr, allocator), control_pair_(count_allocator_type(allocator), Destroyer()) {}

    constexpr explicit shared_storage(const Ty* ptr, const Destroyer& destroyer)
        : pair_(ptr, allocator_type{}), control_pair_(count_allocator_type(), destroyer) {}

    constexpr explicit shared_storage(
        const Ty* ptr, const Allocator& allocator, const Destroyer& destroyer
    )
        : pair_(ptr, allocator), control_pair_(count_allocator_type(allocator), destroyer) {}

    template <typename... Args>
    requires std::is_constructible_v<Ty, Args...>
    explicit shared_storage(Args&&... args)
        : pair_(allocator_type().allocate(), allocator_type()),
          control_pair_(count_allocator_type().allocate(), count_allocator_type()) {
        try {
            ::new (pair_.first()) Ty(std::forward<Args>(args)...);
            pair_.first() = std::launder(pair_.first());
        }
        catch (...) {
            control_pair_.first() = nullptr;
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
    shared_storage(const shared_storage<T>& that) noexcept
        : pair_(that.pair_), control_pair_(that.control_pair_) {
        inc();
    }

    template <typename T>
    shared_storage(shared_storage<T>&& that) noexcept
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
    shared_storage& operator=(const shared_storage<T>& that) noexcept {
        // this != &that
        dec();
        pair_         = that.pair_;
        control_pair_ = that.control_pair_;
        inc();

        return *this;
    }

    template <typename T>
    shared_storage& operator=(shared_storage<T>&& that) noexcept {
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
            count_ = ::new count_type(1);
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
                auto& ptr_       = pair_.first();
                auto& allocator_ = pair_.second();
                auto& counter_   = control_pair_.first();
                auto& destroyer_ = control_pair_.second();
                destroyer_(ptr_);
                allocator_.deallocate(ptr_);
                count_allocator_type(allocator_).deallocate(counter_);
                ptr_     = nullptr;
                counter_ = nullptr;
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
