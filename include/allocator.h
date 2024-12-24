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

constexpr const auto min_align = 8ui64;

} // namespace internal

template <typename Ty, concepts::memory_pool MemoryPool>
class allocator;

template <typename Ty, concepts::memory_pool MemoryPool>
class allocator {
    template <typename, concepts::memory_pool>
    friend class allocator;

public:
    using value_type      = Ty;
    using pointer         = Ty*;
    using const_pointer   = const Ty&;
    using reference       = Ty&;
    using const_reference = const Ty&;

    using shared_type = typename MemoryPool::shared_type;

    template <typename Other, concepts::memory_pool Pool = MemoryPool>
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

    constexpr ~allocator() = default;

    template <typename Other, concepts::memory_pool Pool = MemoryPool>
    explicit constexpr allocator(const allocator<Other, Pool>& that
    ) noexcept(std::is_nothrow_copy_constructible_v<shared_type>)
        : pool_(that.pool_) {}

    template <typename Other, concepts::memory_pool Pool = MemoryPool>
    explicit constexpr allocator(allocator<Other, Pool>&& that
    ) noexcept(std::is_nothrow_move_constructible_v<shared_type>)
        : pool_(std::move(that.pool_)) {}

    [[nodiscard]] auto allocate(const size_t count = 1) -> Ty* {
        return pool_->template allocate<Ty>(
            count, static_cast<std::align_val_t>(std::min(alignof(Ty), internal::min_align))
        );
    }

    constexpr void deallocate(Ty* const ptr, const size_t count = 1) {
        pool_->template deallocate<Ty>(
            ptr, count, static_cast<std::align_val_t>(std::min(alignof(Ty), internal::min_align))
        );
    }

    [[nodiscard]] bool operator==(const allocator& that) const noexcept {
        return pool_ == that.pool_;
    }

private:
    shared_type pool_;
};

template <typename Ty, std::size_t N, concepts::memory_pool MemoryPool>
class allocator<Ty[N], MemoryPool> {
public:
    using shared_type = typename MemoryPool::shared_type;

    using value_type      = Ty*;
    using pointer         = Ty**;
    using const_pointer   = const Ty**;
    using reference       = Ty**&;
    using const_reference = Ty** const&;

    template <typename Other, concepts::memory_pool Pool = MemoryPool>
    using rebind_t = allocator<Other, Pool>;

    constexpr allocator() noexcept                            = default;
    constexpr allocator(const allocator&) noexcept            = default;
    constexpr allocator(allocator&&) noexcept                 = default;
    constexpr allocator& operator=(const allocator&) noexcept = default;
    constexpr allocator& operator=(allocator&&) noexcept      = default;
    constexpr ~allocator() noexcept                           = default;

    template <typename Other, concepts::memory_pool Pool = MemoryPool>
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

template <concepts::completed_type Ty, size_t Count = 1>
class builtin_storage_allocator;

template <concepts::completed_type Ty, size_t Count>
class builtin_storage_allocator {
public:
    using value_type      = Ty;
    using pointer         = Ty*;
    using const_pointer   = const Ty*;
    using reference       = Ty&;
    using const_reference = const Ty&;

    template <typename Other, size_t Count_ = 1>
    using rebind_t = builtin_storage_allocator<Other, Count_>;

    constexpr builtin_storage_allocator() = default;
    constexpr builtin_storage_allocator(const builtin_storage_allocator&) noexcept {}
    constexpr builtin_storage_allocator(builtin_storage_allocator&&) noexcept {}
    
    // NOLINTBEGIN(bugprone-unhandled-self-assignment)
    
    constexpr builtin_storage_allocator& operator=(const builtin_storage_allocator&) noexcept {
        return *this;
    }
    
    // NOLINTEND(bugprone-unhandled-self-assignment)
    
    constexpr builtin_storage_allocator& operator=(builtin_storage_allocator&&) noexcept {
        return *this;
    }
    constexpr ~builtin_storage_allocator() noexcept = default;

    template <typename Other>
    explicit constexpr builtin_storage_allocator(const builtin_storage_allocator<Other>& that) {}

    constexpr auto allocate() const noexcept -> Ty* { return static_cast<Ty*>(&storage_); }

    constexpr void deallocate() const noexcept {}

private:
    std::aligned_storage_t<sizeof(Ty), alignof(Ty)> storage_[Count];
};

} // namespace atom::utils
