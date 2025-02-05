#pragma once
#include <map>
#include <memory>
#include <stack>
#include "core.hpp"

namespace atom::utils {

namespace internal {

const std::align_val_t default_align{ 16 };

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
public:
    using self_type = synchronized_pool;
    class pool;
    using shared_type = std::shared_ptr<pool>;

    using size_type = std::size_t;

    class pool {
        using memory_block = ::atom::utils::internal::memory_block;

        friend class synchronized_pool;

        pool() = default;

        static auto get() -> std::shared_ptr<pool> { return std::shared_ptr<pool>(); }

    public:
        using size_type = synchronized_pool::size_type;

        pool(const pool&)            = delete;
        pool& operator=(const pool&) = delete;

        pool(pool&& that)
            : unused_blocks_(std::move(that.unused_blocks_)),
              using_blocks_(std::move(that.using_blocks_)) {}

        pool& operator=(pool&& that) {
            if (this != &that) {
                unused_blocks_ = std::move(that.unused_blocks_);
                using_blocks_  = std::move(that.using_blocks_);
            }
            return *this;
        }

        ~pool() {}

        template <typename Ty>
        auto allocate(
            const size_type count = 1, const std::align_val_t align = std::align_val_t{ 16 }
        ) -> Ty* {
            // TODO: allocate impl
            return static_cast<Ty*>(operator new(sizeof(Ty) * count, align));
        }

        template <typename Ty>
        void deallocate(
            Ty* ptr,
            const size_type count        = 1,
            const std::align_val_t align = std::align_val_t{ 16 }
        ) {
            // TODO: deallocate impl
            operator delete(static_cast<void*>(ptr), sizeof(Ty) * count, align);
        }

    private:
        std::map<size_type, std::stack<memory_block>> unused_blocks_;
        std::map<size_type, std::map<default_id_t, memory_block>> using_blocks_;
    };

    synchronized_pool() : pool_(pool::get()) {}
    synchronized_pool(const synchronized_pool&)            = delete;
    synchronized_pool& operator=(const synchronized_pool&) = delete;
    synchronized_pool(synchronized_pool&& that) : pool_(std::move(that.pool_)) {}
    synchronized_pool& operator=(synchronized_pool&& that) {
        if (this != &that) {
            pool_ = std::move(that.pool_);
        }
        return *this;
    }
    ~synchronized_pool() = default;

    auto get() -> shared_type { return pool_; }
    auto get() const -> const shared_type { return pool_; }

private:
    shared_type pool_;
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
        auto allocate(
            const size_type count = 1, const std::align_val_t align = std::align_val_t{ 16 }
        ) -> Ty* {
            // TODO: allocate impl
            return operator new(sizeof(Ty) * count, align);
        }

        template <typename Ty>
        void deallocate(
            Ty* ptr,
            const size_type count        = 1,
            const std::align_val_t align = std::align_val_t{ 16 }
        ) {
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
