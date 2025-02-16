#pragma once
#include <concepts>
#include <shared_mutex>
#include <vector>
#include "core.hpp"
#include "core/pair.hpp"
#include "memory.hpp"
#include "memory/allocator.hpp"
#include "memory/storage.hpp"
#include "structures.hpp"

namespace atom::utils {
template <
    std::unsigned_integral Ty, ::atom::utils::concepts::rebindable_allocator Alloc,
    std::size_t PageSize>
class dense_set {
    template <typename Target>
    using allocator_t = typename ::atom::utils::rebind_allocator<Alloc>::template to<Target>::type;

public:
    using value_type      = Ty;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type  = allocator_t<value_type>;

    using iterator = typename std::vector<value_type, allocator_t<value_type>>::iterator;
    using const_iterator =
        typename std::vector<value_type, allocator_t<value_type>>::const_iterator;

private:
    using array_t   = std::array<size_type, PageSize>;
    using storage_t = ::atom::utils::unique_storage<array_t, allocator_t<array_t>>;

public:
    dense_set() : alloc_n_dense_(), sparse_(), dense_mutex_(), sparse_mutex_() {}

    template <typename Al>
    requires std::is_constructible_v<allocator_t<value_type>, Al> &&
                 std::is_constructible_v<allocator_t<array_t>, Al> &&
                 std::is_constructible_v<allocator_t<storage_t>, Al>
    dense_set(Al&& allocator)
        : alloc_n_dense_(allocator_t<value_type>(allocator), allocator_t<value_type>(allocator)),
          sparse_(allocator_t<storage_t>(std::forward<Al>(allocator))) {}

    ~dense_set();

    bool contains(const value_type val) const noexcept {}

    template <typename... Args>
    requires std::is_constructible_v<value_type, Args...>
    void emplace(Args&&... args) {}

    [[nodiscard]] const_iterator find(const value_type val) const noexcept {}

    const_iterator erase(const value_type val) noexcept {}

    const_iterator erase(const_iterator iter) noexcept {}

private:
    ::atom::utils::compressed_pair<
        allocator_t<value_type>, std::vector<Ty, allocator_t<value_type>>>
        alloc_n_dense_;
    std::vector<storage_t, allocator_t<storage_t>> sparse_;
    std::shared_mutex dense_mutex_;
    std::shared_mutex sparse_mutex_;
};

template <std::unsigned_integral Ty, std::size_t PageSize = k_default_page_size>
using sync_dense_set = dense_set<Ty, sync_allocator<Ty>, PageSize>;

template <std::unsigned_integral Ty>
using sync_dense_set_allocator = sync_allocator<Ty>;

template <std::unsigned_integral Ty, std::size_t PageSize = k_default_page_size>
using unsync_dense_set = dense_set<Ty, unsync_allocator<Ty>, PageSize>;

template <std::unsigned_integral Ty>
using unsync_dense_set_allocator = unsync_allocator<Ty>;

} // namespace atom::utils
