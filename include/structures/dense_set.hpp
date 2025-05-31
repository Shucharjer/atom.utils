#pragma once
#include <concepts>
#include <memory>
#include <shared_mutex>
#include <vector>
#include "core.hpp"
#include "core/pair.hpp"
#include "memory.hpp"
#include "memory/allocator.hpp"
#include "memory/storage.hpp"
#include "structures.hpp"

namespace atom::utils {
template <std::unsigned_integral Ty, typename Alloc, std::size_t PageSize>
class dense_set {
    template <typename Target>
    using allocator_t = typename ::atom::utils::rebind_allocator<Alloc>::template to<Target>::type;

    using alty        = allocator_t<Ty>;
    using alty_traits = std::allocator_traits<alty>;

public:
    using value_type      = Ty;
    using pointer         = typename alty_traits::pointer;
    using const_pointer   = typename alty_traits::const_pointer;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using size_type       = typename alty_traits::size_type;
    using difference_type = typename alty_traits::difference_type;
    using allocator_type  = allocator_t<value_type>;

    using iterator = typename std::vector<value_type, allocator_t<value_type>>::iterator;
    using const_iterator =
        typename std::vector<value_type, allocator_t<value_type>>::const_iterator;

private:
    using array_t   = std::array<size_type, PageSize>;
    using storage_t = ::atom::utils::unique_storage<array_t, allocator_t<array_t>>;

public:
    dense_set() : dense_(), sparse_(), dense_mutex_(), sparse_mutex_() {}

    template <typename Al>
    requires std::is_constructible_v<allocator_t<value_type>, Al> &&
                 std::is_constructible_v<allocator_t<storage_t>, Al>
    dense_set(const Al& alloc) : dense_(alloc), sparse_(alloc) {}

    template <typename Al>
    dense_set(std::allocator_arg_t, const Al& alloc) : dense_(alloc), sparse_(alloc) {}

    dense_set(const dense_set&) = default;

    dense_set(dense_set&& that) noexcept : dense_(std::move(that.dense_)), sparse_(that.sparse_) {}

    dense_set& operator=(const dense_set&) = default;

    dense_set& operator=(dense_set&& that) noexcept {
        if (this != &that) [[likely]] {
            dense_  = std::move(that.dense_);
            sparse_ = std::move(that.sparse_);
        }
        return *this;
    }

    ~dense_set();

    bool contains(const value_type val) const noexcept {}

    template <typename... Args>
    requires std::is_constructible_v<value_type, Args...>
    void emplace(Args&&... args) {}

    [[nodiscard]] iterator find(const value_type val) noexcept {}

    [[nodiscard]] const_iterator find(const value_type val) const noexcept {}

    const_iterator erase(const value_type val) noexcept {}

    const_iterator erase(const_iterator iter) noexcept {}

private:
    std::vector<value_type, allocator_type> dense_;
    std::vector<storage_t, allocator_t<storage_t>> sparse_;
    std::shared_mutex dense_mutex_;
    std::shared_mutex sparse_mutex_;
};

} // namespace atom::utils
