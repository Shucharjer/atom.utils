#pragma once
#include <cstdint>
#include <memory>
#include <type_traits>
#include "concepts.h"
#include "type_traits.h"
#include "./fwd.h"

namespace atom::utils {

namespace internal {
#ifdef _WIN32
    #define ALLOCATOR_DECLSPEC __declspec(allocator)
#else
    #define ALLOCATOR_DECLSPEC
#endif

static bool is_aligned(void* ptr, const std::size_t alignment) {
    const uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    return !(addr & (alignment - 1));
}

constexpr const auto min_align = 16ui64;

} // namespace internal

struct basic_allocator {
    using size_type = std::size_t;

    basic_allocator() noexcept                                  = default;
    basic_allocator(const basic_allocator&) noexcept            = default;
    basic_allocator(basic_allocator&&) noexcept                 = default;
    basic_allocator& operator=(const basic_allocator&) noexcept = default;
    basic_allocator& operator=(basic_allocator&&) noexcept      = default;
    virtual ~basic_allocator()                                  = default;

    [[nodiscard]] virtual auto alloc(const std::size_t count = 1) -> void* { return nullptr; }

    virtual auto dealloc(void* ptr, const std::size_t count = 1) noexcept -> void {}

    virtual auto destroy(void* ptr) const -> void {}
};

template <typename Ty>
struct standard_allocator final : public basic_allocator {
    using value_type      = Ty;
    using size_type       = typename basic_allocator::size_type;
    using pointer         = Ty*;
    using const_pointer   = const Ty&;
    using reference       = Ty&;
    using const_reference = const Ty&;

    template <typename T>
    using rebind_t = standard_allocator<T>;

    standard_allocator() noexcept = default;
    standard_allocator(const standard_allocator&) noexcept = default;
    standard_allocator(standard_allocator&&) noexcept = default;
    standard_allocator& operator=(const standard_allocator&) noexcept = default;
    standard_allocator& operator=(standard_allocator&&) noexcept = default;
    ~standard_allocator() noexcept override = default;

    template <typename T>
    constexpr explicit standard_allocator(const standard_allocator<T>&) noexcept {}

    template <typename T>
    constexpr explicit standard_allocator(standard_allocator<T>&&) noexcept {}

    [[nodiscard]] auto alloc(const size_type count = 1) noexcept -> void* override {
        return static_cast<void*>(allocate(count));
    }

    auto dealloc(void* ptr, const size_type count = 1) noexcept -> void override {
        deallocate(static_cast<Ty*>(ptr), count);
    }

    auto destroy(void* ptr) const noexcept(std::is_nothrow_destructible_v<Ty>) -> void override {
        static_cast<Ty*>(ptr)->~Ty();
    }

    [[nodiscard]] Ty* allocate(const size_type count = 1) {
        return std::allocator<Ty>{}.allocate(count);
    }

    void deallocate(Ty* ptr, const size_type count = 1) noexcept {
        std::allocator<Ty>{}.deallocate(ptr, count);
    }
};

template <typename Ty, UCONCEPTS memory_pool MemoryPool>
class allocator;

template <typename Ty, UCONCEPTS memory_pool MemoryPool>
class allocator final : public basic_allocator {
    template <typename, UCONCEPTS memory_pool>
    friend class allocator;

public:
    using value_type      = Ty;
    using size_type       = typename basic_allocator::size_type;
    using pointer         = Ty*;
    using const_pointer   = const Ty&;
    using reference       = Ty&;
    using const_reference = const Ty&;

    using shared_type = typename MemoryPool::shared_type;

    template <typename Other, UCONCEPTS memory_pool Pool = MemoryPool>
    using rebind_t = allocator<Other, Pool>;

    explicit constexpr allocator(const shared_type& pool
    ) noexcept(std::is_nothrow_copy_constructible_v<shared_type>)
        : pool_(pool) {}

    constexpr allocator(const allocator&) noexcept            = default;
    constexpr allocator(allocator&&) noexcept                 = default;
    constexpr allocator& operator=(const allocator&) noexcept = default;
    constexpr allocator& operator=(allocator&& that
    ) noexcept(std::is_nothrow_move_constructible_v<shared_type>) {
        if (this != &that) {
            pool_ = std::move(that.pool_);
        }

        return *this;
    }

    constexpr ~allocator() override = default;

    template <typename Other, UCONCEPTS memory_pool Pool = MemoryPool>
    explicit constexpr allocator(const allocator<Other, Pool>& that
    ) noexcept(std::is_nothrow_copy_constructible_v<shared_type>)
        : pool_(that.pool_) {}

    template <typename Other, UCONCEPTS memory_pool Pool = MemoryPool>
    explicit constexpr allocator(allocator<Other, Pool>&& that
    ) noexcept(std::is_nothrow_move_constructible_v<shared_type>)
        : pool_(std::move(that.pool_)) {}

    [[nodiscard]] auto allocate(const size_t count = 1) -> Ty* {
        return pool_->template allocate<Ty>(
            count, static_cast<std::align_val_t>(std::min(alignof(Ty), UTILS internal::min_align))
        );
    }

    constexpr void deallocate(Ty* const ptr, const size_t count = 1) {
        pool_->template deallocate<Ty>(
            ptr,
            count,
            static_cast<std::align_val_t>(std::min(alignof(Ty), UTILS internal::min_align))
        );
    }

    template <typename... Args>
    constexpr void construct(
        Ty* const ptr, Args&&... args
    ) noexcept(std::is_nothrow_constructible_v<Ty, Args...>) {
        ::new (ptr) Ty(std::forward<Args>(args)...);
    }

    constexpr void destroy(Ty* const ptr) noexcept(std::is_nothrow_destructible_v<Ty>) {
        ptr->~Ty();
    }

    [[nodiscard]] bool operator==(const allocator& that) const noexcept {
        return pool_ == that.pool_;
    }

    [[nodiscard]] auto alloc(const size_type count = 1) noexcept -> void* override {
        return static_cast<void*>(allocate(count));
    }

    auto dealloc(void* ptr, const size_type count = 1) noexcept -> void override {
        deallocate(static_cast<Ty*>(ptr), count);
    }

    auto destroy(void* ptr) const noexcept(std::is_nothrow_destructible_v<Ty>) -> void override {
        static_cast<Ty*>(ptr)->~Ty();
    }

    void destroy(Ty* ptr) const noexcept(std::is_nothrow_destructible_v<Ty>) { ptr->~Ty(); }

private:
    shared_type pool_;
};

template <typename Ty, UCONCEPTS memory_pool MemoryPool>
class allocator<Ty[], MemoryPool> final : public basic_allocator{
public:
    using shared_type = typename MemoryPool::shared_type;

    using value_type = Ty*;
    using pointer = Ty**;
    using const_pointer = const Ty**;
    using reference = Ty*&;
    using const_reference = const Ty*&;

    template <typename Other, UCONCEPTS memory_pool Pool = MemoryPool>
    using rebind_t = allocator<Other, Pool>;
    
};

template <typename Ty, std::size_t N, UCONCEPTS memory_pool MemoryPool>
class allocator<Ty[N], MemoryPool> final : public basic_allocator {
public:
    using shared_type = typename MemoryPool::shared_type;

    using value_type      = Ty*;
    using pointer         = Ty**;
    using const_pointer   = const Ty**;
    using reference       = Ty**&;
    using const_reference = Ty** const&;

    template <typename Other, UCONCEPTS memory_pool Pool = MemoryPool>
    using rebind_t = allocator<Other, Pool>;

    constexpr allocator() noexcept                            = default;
    constexpr allocator(const allocator&) noexcept            = default;
    constexpr allocator(allocator&&) noexcept                 = default;
    constexpr allocator& operator=(const allocator&) noexcept = default;
    constexpr allocator& operator=(allocator&&) noexcept      = default;
    constexpr ~allocator() noexcept override                  = default;

    template <typename Other, UCONCEPTS memory_pool Pool = MemoryPool>
    explicit constexpr allocator(const allocator<Other, Pool>& that
    ) noexcept(std::is_nothrow_copy_constructible_v<Ty>)
        : pool_(that.pool_) {}

    [[nodiscard]] auto allocate(const size_t count = 1) -> Ty** {
        static_assert(sizeof(Ty));

        return pool_->template allocate<Ty>(
            count, static_cast<std::align_val_t>(std::min(alignof(Ty), internal::min_align))
        );
    }

    void deallocate(Ty** const ptr, const size_t count = 1) noexcept {
        return pool_->template deallocate<Ty>(
            ptr, count, static_cast<std::align_val_t>(std::min(alignof(Ty), internal::min_align))
        );
    }

    [[nodiscard]] auto alloc(const size_type count = 1) -> void* override {
        return static_cast<Ty**>(allocate(count));
    }

    auto dealloc(void** ptr, const size_type count = 1) noexcept -> void override {
        deallocate(static_cast<Ty**>(ptr), count);
    }

    auto destroy(Ty** ptr) const noexcept -> void override {}

private:
    shared_type pool_;
};

// template <typename Ty>
// class allocator<Ty, void> : public std::allocator<Ty> {
// public:
//     template <typename Other>
//     using rebind_t = allocator<Other, void>;
//
//     template <typename Other>
//     constexpr allocator(const allocator<Other, void>& that) {}
// };

template <UCONCEPTS completed_type Ty, size_t Count = 1>
class builtin_storage_allocator;

template <UCONCEPTS completed_type Ty, size_t Count>
class builtin_storage_allocator final : public basic_allocator {
public:
    using value_type      = Ty;
    using pointer         = Ty*;
    using const_pointer   = const Ty*;
    using reference       = Ty&;
    using const_reference = const Ty&;

    template <typename Other, size_t Count_ = 1>
    using rebind_t = builtin_storage_allocator<Other, Count_>;

    constexpr builtin_storage_allocator() = default;
    constexpr builtin_storage_allocator(const builtin_storage_allocator&) noexcept : storage_() {}
    constexpr builtin_storage_allocator(builtin_storage_allocator&&) noexcept : storage_() {}

    // NOLINTBEGIN(bugprone-unhandled-self-assignment)

    constexpr builtin_storage_allocator& operator=(const builtin_storage_allocator&) noexcept {
        return *this;
    }

    // NOLINTEND(bugprone-unhandled-self-assignment)

    constexpr builtin_storage_allocator& operator=(builtin_storage_allocator&&) noexcept {
        return *this;
    }
    constexpr ~builtin_storage_allocator() noexcept override  = default;

    template <typename Other>
    constexpr explicit builtin_storage_allocator(const builtin_storage_allocator<Other>&) noexcept : storage_() {}

    constexpr auto allocate() const noexcept -> Ty* { return static_cast<Ty*>(&storage_); }

    constexpr void deallocate() const noexcept {}

    [[nodiscard]] constexpr auto alloc(const size_type count = 1) noexcept -> void* override {
        return static_cast<void*>(storage_);
    }

    constexpr auto dealloc(void* ptr, const size_type count = 1) noexcept -> void override {}

    constexpr auto destroy(void* ptr) const noexcept(std::is_nothrow_destructible_v<Ty>)
        -> void override {
        static_cast<Ty*>(ptr)->~Ty();
    }

private:
    alignas(Ty) std::byte storage_[sizeof(Ty) * Count];
};

template <typename>
struct rebind_allocator;

template <template <typename...> typename Allocator, typename Ty, typename... Others>
struct rebind_allocator<Allocator<Ty, Others...>> {
    template <typename Another>
    struct to {
        using type = Allocator<Another, Others...>;
    };

    template <typename Other>
    using to_t = typename to<Other>::type;
};

} // namespace atom::utils
