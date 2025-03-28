#pragma once
#include <cassert>
#include <map>
#include <memory>
#include <mutex>
#include <stack>
#include "core.hpp"
#include "core/langdef.hpp"

namespace atom::utils {

namespace internal {

const std::align_val_t default_align{ 16 };
const std::size_t default_size = 4096;

struct memory_block {
    std::size_t size;
    void* data;
};

template <typename SizeTy>
constexpr bool is_pow_of_two(const SizeTy size) {
    return size > 0 && (size & (size - 1)) == 0;
}

} // namespace internal

class synchronized_pool {
    using self_type    = synchronized_pool;
    using memory_block = internal::memory_block;

public:
    using shared_type = synchronized_pool*;
    using size_type   = std::size_t;

    synchronized_pool(size_type block_size = internal::default_size) {}

    synchronized_pool(const synchronized_pool&)            = delete;
    synchronized_pool& operator=(const synchronized_pool&) = delete;
    synchronized_pool(synchronized_pool&& that)            = delete;
    synchronized_pool& operator=(synchronized_pool&& that) = delete;
    ~synchronized_pool() {}

    auto get() -> shared_type { return this; }

    template <typename Ty>
    ALLOCATOR auto allocate(
        const size_type count = 1, const std::align_val_t align = std::align_val_t{ 16 }) -> Ty* {
        return static_cast<Ty*>(operator new(sizeof(Ty) * count, align));
    }

    template <typename Ty>
    void deallocate(
        Ty* const ptr, const size_type count = 1,
        const std::align_val_t align = std::align_val_t{ 16 }) noexcept {
        operator delete(ptr, align);
    }

private:
};

class unsynchronized_pool {
public:
    using self_type = unsynchronized_pool;
    class pool;
    using shared_type = std::shared_ptr<pool>;

    using size_type = std::size_t;

    class pool {
        using memory_block = ::atom::utils::internal::memory_block;

        friend class unsynchronized_pool;

        pool() = default;

        static auto get() -> std::shared_ptr<pool>;

    public:
        using size_type = unsynchronized_pool::size_type;

        template <typename Ty>
        ALLOCATOR auto allocate(
            const size_type count = 1, const std::align_val_t align = std::align_val_t{ 16 })
            -> Ty* {
            // TODO: allocate impl
            return operator new(sizeof(Ty) * count, align);
        }

        template <typename Ty>
        void deallocate(
            Ty* ptr, const size_type count = 1,
            const std::align_val_t align = std::align_val_t{ 16 }) {
            // TODO: deallocate impl
            operator delete(ptr, sizeof(Ty) * count, align);
        }

    private:
        std::map<size_type, std::stack<memory_block>> unused_blocks_;
        std::map<size_type, std::map<default_id_t, memory_block>> using_blocks_;
    };

    unsynchronized_pool() : pool_(pool::get()) {}

    unsynchronized_pool(const unsynchronized_pool&)            = default;
    unsynchronized_pool& operator=(const unsynchronized_pool&) = default;

    unsynchronized_pool(unsynchronized_pool&& that) noexcept : pool_(std::move(that.pool_)) {}
    unsynchronized_pool& operator=(unsynchronized_pool&& that) noexcept {
        if (this != &that) {
            pool_ = std::move(that.pool_);
        }
        return *this;
    }

    ~unsynchronized_pool() = default;

    auto get() -> shared_type { return pool_; }
    auto get() const -> const shared_type { return pool_; }

private:
    shared_type pool_;
};

} // namespace atom::utils
