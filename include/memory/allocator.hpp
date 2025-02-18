#pragma once
#include <cassert>
#include <cstdint>
#include <memory>
#include <type_traits>
#include "concepts/mempool.hpp"
#include "concepts/type.hpp"
#include "memory.hpp"

namespace atom::utils {

namespace internal {
#ifdef _WIN32
    #define ALLOCATOR_DECLSPEC __declspec(allocator)
#else
    #define ALLOCATOR_DECLSPEC
#endif

static bool is_aligned(void* ptr, const std::size_t alignment) {
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto addr = reinterpret_cast<uintptr_t>(ptr);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    return !(addr & (alignment - 1));
}

constexpr const auto min_align = 16ULL;

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
};

template <typename Ty>
struct standard_allocator : public basic_allocator {
    using value_type      = Ty;
    using size_type       = typename basic_allocator::size_type;
    using pointer         = Ty*;
    using const_pointer   = const Ty&;
    using reference       = Ty&;
    using const_reference = const Ty&;

    template <typename T>
    using rebind_t = standard_allocator<T>;

    static_assert(!std::is_const_v<Ty>);
    static_assert(!std::is_function_v<Ty>);
    static_assert(!std::is_reference_v<Ty>);

    standard_allocator() noexcept                                     = default;
    standard_allocator(const standard_allocator&) noexcept            = default;
    standard_allocator(standard_allocator&&) noexcept                 = default;
    standard_allocator& operator=(const standard_allocator&) noexcept = default;
    standard_allocator& operator=(standard_allocator&&) noexcept      = default;
    ~standard_allocator() noexcept override                           = default;

    template <typename T>
    constexpr explicit standard_allocator(const standard_allocator<T>&) noexcept {}

    // NOLINTBEGIN(cppcoreguidelines-rvalue-reference-param-not-moved)
    template <typename T>
    constexpr explicit standard_allocator(standard_allocator<T>&&) noexcept {}
    // NOLINTEND(cppcoreguidelines-rvalue-reference-param-not-moved)

    [[nodiscard]] auto alloc(const size_type count = 1) noexcept -> void* override {
        return static_cast<void*>(allocate(count));
    }

    auto dealloc(void* ptr, const size_type count = 1) noexcept -> void override {
        deallocate(static_cast<Ty*>(ptr), count);
    }

    [[nodiscard]] constexpr Ty* allocate(const size_type count = 1) {
        return std::allocator<Ty>{}.allocate(count);
    }

    constexpr void deallocate(Ty* ptr, const size_type count = 1) noexcept {
        std::allocator<Ty>{}.deallocate(ptr, count);
    }
};

template <typename Ty, ::atom::utils::concepts::mempool MemoryPool>
class allocator;

template <typename Ty, ::atom::utils::concepts::mempool MemoryPool>
class allocator : public basic_allocator {
    template <typename, ::atom::utils::concepts::mempool>
    friend class allocator;

public:
    using value_type      = Ty;
    using size_type       = typename basic_allocator::size_type;
    using pointer         = Ty*;
    using const_pointer   = const Ty&;
    using reference       = Ty&;
    using const_reference = const Ty&;

    using shared_type = typename MemoryPool::shared_type;

    template <typename Other, ::atom::utils::concepts::mempool Pool = MemoryPool>
    using rebind_t = allocator<Other, Pool>;

    static_assert(!std::is_const_v<Ty>);
    static_assert(!std::is_function_v<Ty>);
    static_assert(!std::is_reference_v<Ty>);

    explicit constexpr allocator(const shared_type& pool) noexcept(
        std::is_nothrow_copy_constructible_v<shared_type>)
        : pool_(pool) {}

    explicit constexpr allocator(MemoryPool& pool) noexcept(
        std::is_nothrow_copy_constructible_v<shared_type>)
    requires requires(MemoryPool pool) {
        pool.get();
        { pool.get() } -> std::same_as<shared_type>;
    }
        : pool_(pool.get()) {}

    constexpr allocator(const allocator&) noexcept            = default;
    constexpr allocator(allocator&&) noexcept                 = default;
    constexpr allocator& operator=(const allocator&) noexcept = default;
    constexpr allocator& operator=(allocator&& that) noexcept(
        std::is_nothrow_move_constructible_v<shared_type>) {
        if (this != &that) {
            pool_ = std::move(that.pool_);
        }

        return *this;
    }

    constexpr ~allocator() override = default;

    template <typename Other, ::atom::utils::concepts::mempool Pool = MemoryPool>
    explicit constexpr allocator(const allocator<Other, Pool>& that) noexcept(
        std::is_nothrow_copy_constructible_v<shared_type>)
        : pool_(that.pool_) {}

    // NOLINTBEGIN(cppcoreguidelines-rvalue-reference-param-not-moved)
    template <typename Other, ::atom::utils::concepts::mempool Pool = MemoryPool>
    explicit constexpr allocator(allocator<Other, Pool>&& that) noexcept(
        std::is_nothrow_move_constructible_v<shared_type>)
        : pool_(std::move(that.pool_)) {}
    // NOLINTEND(cppcoreguidelines-rvalue-reference-param-not-moved)

    [[nodiscard]] auto allocate(const size_t count = 1) -> Ty* {
        return pool_->template allocate<Ty>(
            count, static_cast<std::align_val_t>(
                       std::min(alignof(Ty), ::atom::utils::internal::min_align)));
    }

    constexpr void deallocate(Ty* const ptr, const size_t count = 1) {
        pool_->template deallocate<Ty>(
            ptr, count,
            static_cast<std::align_val_t>(
                std::min(alignof(Ty), ::atom::utils::internal::min_align)));
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

private:
    shared_type pool_;
};

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
template <typename Ty, ::atom::utils::concepts::mempool MemoryPool>
class allocator<Ty[], MemoryPool> final : public basic_allocator {
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays)
public:
    using shared_type = typename MemoryPool::shared_type;

    using value_type      = Ty*;
    using pointer         = Ty**;
    using const_pointer   = const Ty**;
    using reference       = Ty*&;
    using const_reference = const Ty*&;

    template <typename Other, ::atom::utils::concepts::mempool Pool = MemoryPool>
    using rebind_t = allocator<Other, Pool>;
};

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
template <typename Ty, std::size_t N, ::atom::utils::concepts::mempool MemoryPool>
class allocator<Ty[N], MemoryPool> final : public basic_allocator {
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays)
public:
    using shared_type = typename MemoryPool::shared_type;

    using value_type      = Ty*;
    using pointer         = Ty**;
    using const_pointer   = const Ty**;
    using reference       = Ty**&;
    using const_reference = Ty** const&;

    template <typename Other, ::atom::utils::concepts::mempool Pool = MemoryPool>
    using rebind_t = allocator<Other, Pool>;

    constexpr allocator() noexcept                            = default;
    constexpr allocator(const allocator&) noexcept            = default;
    constexpr allocator(allocator&&) noexcept                 = default;
    constexpr allocator& operator=(const allocator&) noexcept = default;
    constexpr allocator& operator=(allocator&&) noexcept      = default;
    constexpr ~allocator() noexcept override                  = default;

    template <typename Other, ::atom::utils::concepts::mempool Pool = MemoryPool>
    explicit constexpr allocator(const allocator<Other, Pool>& that) noexcept(
        std::is_nothrow_copy_constructible_v<Ty>)
        : pool_(that.pool_) {}

    [[nodiscard]] auto allocate(const size_t count = 1) -> Ty** {
        static_assert(sizeof(Ty));

        return pool_->template allocate<Ty>(
            count, static_cast<std::align_val_t>(std::min(alignof(Ty), internal::min_align)));
    }

    void deallocate(Ty** const ptr, const size_t count = 1) noexcept {
        return pool_->template deallocate<Ty>(
            ptr, count, static_cast<std::align_val_t>(std::min(alignof(Ty), internal::min_align)));
    }

    [[nodiscard]] auto alloc(const size_type count = 1) -> void* override {
        return static_cast<Ty**>(allocate(count));
    }

    auto dealloc(void* ptr, const size_type count = 1) noexcept -> void override {
        deallocate(static_cast<Ty**>(ptr), count);
    }

    auto destroy(Ty** ptr) const noexcept -> void override {}

private:
    shared_type pool_;
};

template <::atom::utils::concepts::completed Ty, size_t Count = 1>
class builtin_storage_allocator;

template <::atom::utils::concepts::completed Ty, size_t Count>
class builtin_storage_allocator : public basic_allocator {
public:
    using value_type      = Ty;
    using pointer         = Ty*;
    using const_pointer   = const Ty*;
    using reference       = Ty&;
    using const_reference = const Ty&;

    template <typename Other, size_t Count_ = 1>
    using rebind_t = builtin_storage_allocator<Other, Count_>;

    constexpr builtin_storage_allocator() noexcept : storage_() {};
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
    constexpr ~builtin_storage_allocator() noexcept override = default;

    template <typename Other>
    constexpr explicit builtin_storage_allocator(const builtin_storage_allocator<Other>&) noexcept
        : storage_() {}

    constexpr auto allocate(const size_type count = 1) const noexcept -> Ty* {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        return const_cast<Ty*>(reinterpret_cast<const Ty*>(&storage_));
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    constexpr void deallocate(Ty* ptr, const size_type count = 1) const noexcept {}

    [[nodiscard]] constexpr auto alloc(const size_type count = 1) noexcept -> void* override {
        return static_cast<void*>(storage_);
    }

    constexpr auto dealloc(void* ptr, const size_type count = 1) noexcept -> void override {}

private:
    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
    // NOLINTBEGIN(modernize-avoid-c-arrays)
    alignas(Ty) std::byte storage_[sizeof(Ty) * Count];
    // NOLINTEND(modernize-avoid-c-arrays)
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays)
};

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
struct rebind_allocator<Allocator<Ty, Others...>> {
    template <typename Other>
    struct to {
        using type = Allocator<Other, Others...>;
    };

    template <typename Other>
    using to_t = typename to<Other>::type;
};

} // namespace atom::utils
