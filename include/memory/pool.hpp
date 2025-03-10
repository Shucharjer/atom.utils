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

    static constexpr size_type default_block_size = 4096; // 4KB
    static constexpr size_type min_block_size     = 16;

    synchronized_pool(size_type block_size = default_block_size) : block_size_(block_size) {
        assert(internal::is_pow_of_two(block_size_));
    }

    synchronized_pool(const synchronized_pool&)            = delete;
    synchronized_pool& operator=(const synchronized_pool&) = delete;
    synchronized_pool(synchronized_pool&& that)            = delete;
    synchronized_pool& operator=(synchronized_pool&& that) = delete;
    ~synchronized_pool() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [size, blocks] : unused_blocks_) {
            while (!blocks.empty()) {
                auto block = blocks.top();
                operator delete(block.data, block.size);
                blocks.pop();
            }
        }
        for (auto& [size, blocks] : using_blocks_) {
            for (auto& [id, block] : blocks) {
                operator delete(block.data, block.size);
            }
        }
    }

    auto get() -> shared_type { return this; }

    template <typename Ty>
    ALLOCATOR auto allocate(
        const size_type count = 1, const std::align_val_t align = std::align_val_t{ 16 }) -> Ty* {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto size = sizeof(Ty) * count;
        const auto aligned_size =
            (size + static_cast<size_t>(align) - 1) & ~(static_cast<size_t>(align) - 1);

        // Try to find suitable block in unused blocks
        if (auto it = unused_blocks_.find(aligned_size);
            it != unused_blocks_.end() && !it->second.empty()) {
            auto block = it->second.top();
            it->second.pop();
            auto id                         = next_id_++;
            using_blocks_[aligned_size][id] = block;
            return static_cast<Ty*>(block.data);
        }

        // Allocate new block
        void* ptr = operator new(block_size_, align);
        memory_block block{ block_size_, ptr };
        auto id                         = next_id_++;
        using_blocks_[aligned_size][id] = block;
        return static_cast<Ty*>(ptr);
    }

    template <typename Ty>
    void deallocate(
        Ty* const ptr, const size_type count = 1,
        const std::align_val_t align = std::align_val_t{ 16 }) noexcept {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto size = sizeof(Ty) * count;
        const auto aligned_size =
            (size + static_cast<size_t>(align) - 1) & ~(static_cast<size_t>(align) - 1);

        // Find block in using blocks
        if (auto it = using_blocks_.find(aligned_size); it != using_blocks_.end()) {
            for (auto block_it = it->second.begin(); block_it != it->second.end(); ++block_it) {
                if (block_it->second.data == ptr) {
                    // Move to unused blocks
                    unused_blocks_[aligned_size].push(block_it->second);
                    it->second.erase(block_it);
                    return;
                }
            }
        }

        // Fallback to direct delete if not found
        operator delete(static_cast<void*>(ptr), size, align);
    }

private:
    std::mutex mutex_;
    size_type block_size_;
    default_id_t next_id_ = 0;
    std::map<size_type, std::stack<memory_block>> unused_blocks_;
    std::map<size_type, std::map<default_id_t, memory_block>> using_blocks_;
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
