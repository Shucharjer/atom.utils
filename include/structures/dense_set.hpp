#pragma once
#include <concepts>
#include <shared_mutex>
#include "core.hpp"
#include "core/pair.hpp"
#include "memory.hpp"
#include "memory/allocator.hpp"
#include "structures.hpp"

namespace atom::utils {
template <std::unsigned_integral Ty, UCONCEPTS rebindable_allocator Alloc, std::size_t PageSize>
class dense_set {
    template <typename Target>
    using allocator_t = typename UTILS rebind_allocator<Alloc>::template to<Target>::type;

public:
    using value_type      = Ty;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type  = allocator_t<value_type>;

private:
    using array_t   = std::array<size_type, PageSize>;
    using storage_t = UTILS unique_storage<array_t, allocator_t<array_t>>;

public:
    dense_set();

    ~dense_set();

private:
    UTILS compressed_pair<allocator_t<value_type>, std::vector<Ty, allocator_t<Ty>>> alloc_n_dense_;
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
