#pragma once
#include <cassert>
#include <cstddef>
#include <iostream>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace atom::utils {
/**
 * @brief Memory pool
 *
 * There will be a new memory pool one day, if I am alive...
 *
 */
class memory_pool {

    struct basic_pool_params {
        std::size_t block_size;
        std::size_t initial_capacity;
    };

    static constexpr std::size_t kDefaultInitialCapacity = 8;

    static constexpr basic_pool_params kDefaultBasicPoolParams =
        basic_pool_params{ .block_size = 1024, .initial_capacity = kDefaultInitialCapacity };

    class basic_pool {
    public:
        /**
         * @brief Construct a new basic pool object
         *
         * @param params Parameters contains the block size and initial capacity of this pool.
         */
        basic_pool(const basic_pool_params& params = kDefaultBasicPoolParams)
            : block_size_(params.block_size), data_(params.initial_capacity, nullptr) {
            for (auto& addr : data_) {
                addr = operator new(block_size_);
            }
        }
        basic_pool(const basic_pool&)            = delete;
        basic_pool(basic_pool&&)                 = delete;
        basic_pool& operator=(const basic_pool&) = delete;
        basic_pool& operator=(basic_pool&&)      = delete;
        /**
         * @brief Destroy the basic pool object
         *
         * Before calling this, you should dealloc each memory block allocated by this.
         *
         * Obversly, you should do this via a `memory_pool` instance.
         *
         */
        ~basic_pool() {
            assert(data_.size() == data_.capacity());

            for (auto& addr : data_) {
                operator delete(addr, block_size_);
            }
        }

        [[nodiscard]] auto empty() const noexcept -> bool { return data_.empty(); }
        [[nodiscard]] auto size() const noexcept -> std::size_t { return data_.size(); }
        [[nodiscard]] auto capacity() const noexcept -> std::size_t { return data_.capacity(); }
        [[nodiscard]] auto block_size() const noexcept -> std::size_t { return block_size_; }

        /**
         * @brief Allocate a block of memory to store data.
         *
         * Please make judgement before calling this function.
         *
         * `noexcept`
         *
         * It will return `nullptr` when allocating failed.
         *
         * @return void* const
         */
        auto alloc(bool expand = true) noexcept -> void* const {
            // when data_ being empty, there is no unused memory

            bool flag     = false;
            auto capacity = data_.capacity();
            std::lock_guard<std::mutex> lock(mutex_);
            if (data_.empty()) [[unlikely]] {
                // if empty, we will try allocate more blocks of memory

                if (expand) [[likely]] {

                    try {
                        data_.reserve(2 * capacity);
                        try {
                            for (std::size_t i = 0; i < capacity; ++i) {
                                data_.push_back(operator new(block_size_));
                            }
                        }
                        catch (...) {
                            for (auto& addr : data_) {
                                operator delete(addr, block_size_);
                            }
                            data_.clear();
                            data_.resize(capacity);
                            throw;
                        }
                    }
                    catch (...) {
                        flag = true;
                    }
                }
                else [[unlikely]] {
                    flag = true;
                }
            }

            if (flag) [[unlikely]] {
                return nullptr;
            }
            else [[likely]] {
                auto ptr = data_.back();
                data_.pop_back();
                return ptr;
            }
        }

        /**
         * @brief Deallocate a block of memory.
         *
         * `noexcept`
         *
         * @param ptr
         */
        void dealloc(void* const ptr) noexcept {
            std::lock_guard<std::mutex> lock(mutex_);
            data_.push_back(ptr);
        }

    private:
        std::size_t block_size_;
        /**
         * @brief Stores pointer not allocated to something
         *
         * Used as a stack that could treverse, so it will not throw a exception when emplacing
         * elements to this container.
         */
        std::vector<void*> data_;
        std::mutex mutex_;
    };

public:
    memory_pool(const std::size_t max_size = 102400) : max_size_(max_size) {}
    memory_pool(const memory_pool& other)            = delete;
    memory_pool& operator=(const memory_pool& other) = delete;
    memory_pool(memory_pool&&)                       = delete;
    memory_pool& operator=(memory_pool&&)            = delete;

    ~memory_pool() { do_delete(); }

    [[nodiscard]] auto max_size() const noexcept -> std::size_t { return max_size_; }
    [[nodiscard]] auto size() const noexcept -> std::size_t { return size_; }

    /**
     * @brief Allocate a block of memory storeing Ty
     *
     * If success, it will return the address;
     * Otherwise, it will return `nullptr`.
     *
     * Basic Guarantee
     *
     * Please check the return value. (Is it a `nullptr`?)
     *
     * @tparam Ty Type to allocate
     * @tparam Args Argument types in construction
     * @param args Arguments
     * @return Ty* const
     */
    template <typename Ty, typename... Args>
    [[nodiscard]] auto alloc(Args&&... args) noexcept(std::is_nothrow_constructible_v<Ty, Args...>)
        -> Ty* const {
        auto block_size = sizeof(Ty);
        std::shared_lock<std::shared_mutex> lock(pool_mutex_);
        std::shared_lock<std::shared_mutex> size_lock(size_mutex_);
        // find basic_pool in pool_
        if (!pools_.contains(block_size)) [[unlikely]] {
            if (block_size * kDefaultInitialCapacity + size_ > max_size_) {
                return nullptr;
            }
            else {
                lock.unlock();
                std::unique_lock<std::shared_mutex> ulock(pool_mutex_);
                try {
                    auto pool = std::make_shared<basic_pool>(kDefaultBasicPoolParams);
                    try {
                        pools_.emplace(block_size, std::move(pool));
                        size_lock.unlock();
                        std::unique_lock<std::shared_mutex> usize_lock(size_mutex_);
                        size_ += block_size * pools_.at(block_size)->capacity();
                        usize_lock.unlock();
                        size_lock.lock();
                    }
                    catch (...) {
                        std::cerr << "Memory allocation failed in memory_pool when emplacing a "
                                     "basic_pool"
                                  << std::endl;
                    }
                }
                catch (...) {
                    std::cerr << "Memory allication failed in memory_pool when calling "
                                 "make_shared<basic_pool>(...)"
                              << std::endl;
                }
                ulock.unlock();
                lock.lock();
            }
        }
        // alloc from basic_pool
        auto& pool    = pools_.at(block_size);
        auto capacity = pool->capacity();
        auto ptr =
            static_cast<Ty* const>(pool->alloc(block_size * (pool->capacity() + size_) <= max_size_)
            );
        size_lock.unlock();
        if (pool->capacity() != capacity) {
            std::unique_lock<std::shared_mutex> usize_lock(size_mutex_);
            size_ += (pool->capacity() - capacity) * block_size;
            usize_lock.unlock();
        }
        lock.unlock();
        // call constructor
        if (static_cast<bool>(ptr)) {
            if constexpr (std::is_nothrow_constructible_v<Ty, Args...>) {
                ::new (ptr) Ty(std::forward<Args>(args)...);
            }
            else {
                try {
                    ::new (ptr) Ty(std::forward<Args>(args)...);
                }
                catch (...) {
                    throw;
                }
            }
        }

        return ptr;
    }

    /**
     * @brief Release a block of memory.
     *
     * It will only release the block, rather than call its destructor by the way.
     *
     * Throw when failed
     *
     * @tparam Ty
     * @param ptr
     */
    template <typename Ty>
    void release(Ty* const ptr) {
        std::lock_guard<std::mutex> lock(vector_mutex_);
        try {
            to_delete_.emplace_back(
                sizeof(Ty),
                static_cast<void* const>(ptr),
                [](void* const ptr) -> void { static_cast<Ty* const>(ptr)->~Ty(); }
            );
        }
        catch (...) {
            std::cerr << "Memory release failed in memory_pool [with Ty = " << typeid(Ty).name()
                      << "]" << std::endl;
            throw;
        }
    }

    /**
     * @brief Deallocate a block of memory
     *
     * It is nothrow callable if Ty is nothrow destructible.
     *
     * @tparam Ty
     * @param ptr
     */
    template <typename Ty>
    void dealloc(Ty* const ptr) noexcept(std::is_nothrow_destructible_v<Ty>) {
        std::shared_lock<std::shared_mutex> lock(pool_mutex_);
        if (auto iter = pools_.find(sizeof(Ty)); iter != pools_.cend()) [[likely]] {
            ptr->~Ty();
            iter->second->dealloc(static_cast<void* const>(ptr));
        }
    }

    void do_delete() {
        std::lock_guard<std::mutex> lock(vector_mutex_);
        for (auto& [size, ptr, dlt] : to_delete_) {
            try {
                dlt(ptr);
            }
            catch (...) {
                // we do not know how to deal with these exceptions.
                // I do hope you could use memory pool correctly.
                throw;
            }
            std::unique_lock<std::shared_mutex> ulock(pool_mutex_);
            pools_.at(size)->dealloc(ptr);
        }
        to_delete_.clear();
    }

private:
    std::size_t size_{};
    mutable std::shared_mutex size_mutex_;

    std::size_t max_size_;

    mutable std::shared_mutex pool_mutex_;
    std::unordered_map<std::size_t, std::shared_ptr<basic_pool>> pools_;

    mutable std::mutex vector_mutex_;
    std::vector<std::tuple<std::size_t, void* const, void (*)(void* const)>> to_delete_;
};
} // namespace atom::utils
