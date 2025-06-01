#pragma once
#include <cassert>
#include <cstdint>
#include <memory>
#include <numeric>
#include <type_traits>
#include "concepts/mempool.hpp"
#include "concepts/type.hpp"
#include "core/langdef.hpp"
#include "memory.hpp"

namespace atom::utils {

namespace internal {

static bool is_aligned(void* ptr, const std::size_t alignment) {
    // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
    const auto addr = reinterpret_cast<uintptr_t>(ptr);
    // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    return !(addr & (alignment - 1));
}

constexpr inline auto min_align = 16ULL;

} // namespace internal

struct NOVTABLE basic_allocator {
    using size_type = std::size_t;

    basic_allocator() noexcept                                  = default;
    basic_allocator(const basic_allocator&) noexcept            = default;
    basic_allocator(basic_allocator&&) noexcept                 = default;
    basic_allocator& operator=(const basic_allocator&) noexcept = default;
    basic_allocator& operator=(basic_allocator&&) noexcept      = default;
    virtual ~basic_allocator()                                  = default;

    [[nodiscard]] virtual auto alloc(const std::size_t count = 1) -> void* { return nullptr; }

    virtual auto dealloc(void* const ptr, const std::size_t count = 1) noexcept -> void {}
};

template <typename Ty>
struct standard_allocator : public basic_allocator, private std::allocator<Ty> {
    template <typename T>
    friend struct standard_allocator;

    using value_type      = Ty;
    using size_type       = typename basic_allocator::size_type;
    using pointer         = Ty*;
    using const_pointer   = const Ty*;
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
    constexpr standard_allocator(const standard_allocator<T>& that) noexcept(
        std::is_nothrow_constructible_v<std::allocator<Ty>, std::allocator<T>>)
        : std::allocator<Ty>(static_cast<const std::allocator<T>&>(that)) {}

    [[nodiscard]] auto alloc(const size_type count = 1) noexcept -> void* override {
        return static_cast<void*>(allocate(count));
    }

    auto dealloc(void* ptr, const size_type count = 1) noexcept -> void override {
        deallocate(static_cast<Ty*>(ptr), count);
    }

    [[nodiscard]] constexpr Ty* allocate(const size_type count = 1) { return allocate(count); }

    constexpr void deallocate(Ty* ptr, const size_type count = 1) noexcept {
        deallocate(ptr, count);
    }

    constexpr bool operator==(const standard_allocator&) const noexcept { return true; }

    constexpr bool operator!=(const standard_allocator&) const noexcept { return false; }

    template <typename T>
    constexpr bool operator==(const standard_allocator<T>&) const noexcept {
        return false;
    }

    template <typename T>
    constexpr bool operator!=(const standard_allocator<T>&) const noexcept {
        return true;
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
    using const_pointer   = const Ty*;
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
                       std::min<std::size_t>(alignof(Ty), ::atom::utils::internal::min_align)));
    }

    constexpr void deallocate(Ty* const ptr, const size_t count = 1) {
        pool_->template deallocate<Ty>(
            ptr, count,
            static_cast<std::align_val_t>(
                std::min<std::size_t>(alignof(Ty), ::atom::utils::internal::min_align)));
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
            count,
            static_cast<std::align_val_t>(std::min<std::size_t>(alignof(Ty), internal::min_align)));
    }

    void deallocate(Ty** const ptr, const size_t count = 1) noexcept {
        return pool_->template deallocate<Ty>(
            ptr, count,
            static_cast<std::align_val_t>(std::min<std::size_t>(alignof(Ty), internal::min_align)));
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

template <concepts::completed Ty, size_t Count = 1>
class builtin_storage_allocator;

template <concepts::completed Ty, size_t Count>
class builtin_storage_allocator : public basic_allocator {
    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays)
    // NOLINTBEGIN(modernize-avoid-c-arrays)
    struct _Storage {
        alignas(alignof(Ty)) std::byte bytes[sizeof(Ty)];
    };
    alignas(alignof(_Storage)) _Storage storage_[Count];
    void* ptrs_[Count];
    size_t begin_{ 0 };
    size_t end_{ Count };

    constexpr static size_t _Mask = Count - 1;

    void _Gen_ptrs() noexcept { std::iota(std::rbegin(ptrs_), std::rend(ptrs_), storage_); }

public:
    using value_type      = Ty;
    using pointer         = Ty*;
    using const_pointer   = const Ty*;
    using reference       = Ty&;
    using const_reference = const Ty&;

    template <typename Other, size_t Count_ = 1>
    using rebind_t = builtin_storage_allocator<Other, Count_>;

    constexpr builtin_storage_allocator() noexcept : storage_() { _Gen_ptrs(); };
    constexpr builtin_storage_allocator(const builtin_storage_allocator&) noexcept : storage_() {
        _Gen_ptrs();
    }
    constexpr builtin_storage_allocator(builtin_storage_allocator&&) noexcept : storage_() {
        _Gen_ptrs();
    }

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

    constexpr auto allocate() noexcept -> Ty* {
        // NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
        return static_cast<Ty*>(ptrs_[begin_++ & _Mask]);
        // NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
    }

    constexpr void deallocate(Ty* const ptr) noexcept {
        ptrs_[end_++ & _Mask] = static_cast<Ty*>(ptr);
    }

    // [[nodiscard]] constexpr auto alloc() noexcept -> void* override {
    //     return static_cast<void*>(storage_);
    // }

    // constexpr auto dealloc(void* const ptr) noexcept -> void override {}

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
