#pragma once
#include <concepts>
#include "allocator.h"
#include "concepts.h"
#include "pair.h"
#include "storage.h"
#include "threads.h"

namespace atom {

constexpr std::size_t k_default_page_size = 32;

template <
    std::unsigned_integral Key,
    typename Val,
    UCONCEPTS rebindable_allocator Alloc =
        UTILS standard_allocator<UTILS compressed_pair<Key, Val>>,
    std::size_t PageSize = k_default_page_size>
class dense_map {
    template <typename Target>
    using allocator_t = typename UTILS rebind_allocator<Alloc>::template to<Target>::type;

public:
    using key_type    = Key;
    using mapped_type = Val;
    using value_type  = UTILS compressed_pair<key_type, mapped_type>;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    using allocator_type = allocator_t<value_type>;

    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;

    using iterator       = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;

private:
    using array_t            = std::array<size_type, PageSize>;
    using storage_t          = UTILS unique_storage<array_t, allocator_t<array_t>>;
    using shared_lock_keeper = UTILS lock_keeper<2, std::shared_mutex, std::shared_lock>;
    using unique_lock_keeper = UTILS lock_keeper<2, std::shared_mutex, std::unique_lock>;

public:
    dense_map()
        : alloc_n_dense_(allocator_t<value_type>{}, allocator_t<value_type>{}),
          sparse_(allocator_t<storage_t>{}) {}

    template <typename Al>
    requires std::is_constructible_v<allocator_t<value_type>, Al> &&
                 std::is_constructible_v<allocator_t<array_t>, Al> &&
                 std::is_constructible_v<allocator_t<storage_t>, Al>
    explicit dense_map(const Al& allocator)
        : alloc_n_dense_(allocator, allocator_t<value_type>{ allocator }),
          sparse_(allocator_t<storage_t>(allocator)) {}

    dense_map(dense_map&& that) noexcept
        : alloc_n_dense_(std::move(that.alloc_n_dense_)), sparse_(std::move(that.sparse_)) {}

    dense_map& operator=(dense_map&& that) noexcept {
        if (this != &that) {
            unique_lock_keeper that_keeper{ that.dense_mutex_, that.sparse_mutex_ };
            unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
            alloc_n_dense_ = std::move(that.alloc_n_dense_);
            sparse_        = std::move(that.sparse_);
        }
        return *this;
    }

    // TODO:
    dense_map(const dense_map&) = delete;

    // TODO:
    dense_map& operator=(const dense_map&) = delete;

    ~dense_map() noexcept = default;

    auto at(const key_type key) -> mapped_type& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return alloc_n_dense_.second()[sparse_[page]->at(offset)].second();
    }

    [[nodiscard]] auto at(const key_type key) const -> const mapped_type& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return alloc_n_dense_.second()[sparse_[page]->at(offset)].second();
    }

    auto operator[](const key_type key) -> mapped_type& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return alloc_n_dense_.second()[sparse_[page]->at(offset)].second();
    }

    auto operator[](const key_type key) const -> const mapped_type& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return alloc_n_dense_.second()[sparse_[page]->at(offset)].second();
    }

    template <typename ValTy>
    void emplace(const key_type& key, ValTy&& val) {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        try {
            check_page(page);

            sparse_[page]->at(offset) = alloc_n_dense_.size();
            try {
                alloc_n_dense_.emplace_back(key, std::forward<ValTy>(val));
            }
            catch (...) {
                sparse_[page]->at(offset) = 0;
            }
        }
        catch (...) {
            pop_page_to(page);
        }
    }

    template <typename... Args>
    requires std::is_constructible_v<value_type, Args...>
    void emplace(Args&&... args) {
        auto val    = value_type(std::forward<Args>(args)...);
        auto page   = page_of(val.first());
        auto offset = offset_of(val.first());

        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        try {
            check_page(page);
            sparse_[page]->at(offset) = alloc_n_dense_.second().size();
            try {
                alloc_n_dense_.second().emplace_back(std::move(val));
            }
            catch (...) {
                sparse_[page]->at(offset) = 0;
            }
        }
        catch (...) {
            pop_page_to(page);
        }
    }

    auto erase(const key_type key) {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        shared_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        if (contains_impl(page, offset)) {
            keeper.run_away();
            erase_without_check_impl(page, offset);
        }
    }

    auto erase_without_check(const key_type key) {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        erase_without_check(page, offset);
    }

    auto contains(const key_type key) -> bool {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        return contains_impl(page, offset);
    }

    [[nodiscard]] auto find(const key_type key) noexcept -> iterator {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        shared_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        if (contains_impl(page, offset)) {
            return alloc_n_dense_.begin() + alloc_n_dense_[sparse_[page]->at(offset)] -
                   alloc_n_dense_.front();
        }
        else {
            return alloc_n_dense_.end();
        }
    }

    [[nodiscard]] auto find(const key_type key) const noexcept -> const_iterator {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        if (contains_impl(page, offset)) {
            return alloc_n_dense_.cbegin() + alloc_n_dense_[sparse_[page]->at(offset)] -
                   alloc_n_dense_.front();
        }
        else {
            return alloc_n_dense_.cend();
        }
    }

    [[nodiscard]] bool empty() const noexcept { return alloc_n_dense_.second().empty(); }

    [[nodiscard]] size_type size() const noexcept { return alloc_n_dense_.second().size(); }

    void clear() noexcept {
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        sparse_.clear();
        alloc_n_dense_.clear();
    }

    auto front() -> mapped_type& { return alloc_n_dense_.front(); }
    [[nodiscard]] auto front() const -> const mapped_type& { return alloc_n_dense_.front(); }

    auto back() -> mapped_type& { return alloc_n_dense_.back(); }
    [[nodiscard]] auto back() const -> const mapped_type& { return alloc_n_dense_.back(); }

    auto begin() noexcept -> iterator { return alloc_n_dense_.begin(); }
    [[nodiscard]] auto begin() const noexcept -> const_iterator { return alloc_n_dense_.cbegin(); }
    [[nodiscard]] auto cbegin() const noexcept -> const_iterator { return alloc_n_dense_.cbegin(); }

    auto end() noexcept -> iterator { return alloc_n_dense_.end(); }
    [[nodiscard]] auto end() const noexcept -> const_iterator { return alloc_n_dense_.cend(); }
    [[nodiscard]] auto cend() const noexcept -> const_iterator { return alloc_n_dense_.cend(); }

    auto rbegin() noexcept -> iterator { return alloc_n_dense_.rbegin(); }
    [[nodiscard]] auto rbegin() const noexcept -> const_iterator { return alloc_n_dense_.rbegin(); }
    [[nodiscard]] auto crbegin() const noexcept -> const_iterator {
        return alloc_n_dense_.crbegin();
    }

    auto rend() noexcept -> iterator { return alloc_n_dense_.rend(); }
    [[nodiscard]] auto rend() const noexcept -> const_iterator { return alloc_n_dense_.rend(); }
    [[nodiscard]] auto crend() const noexcept -> const_iterator { return alloc_n_dense_.crend(); }

private:
    static size_type page_of(const key_type key) noexcept { return key / PageSize; }
    static size_type offset_of(const key_type key) noexcept { return key % PageSize; }
    void check_page(const size_type page) {
        const auto current_page = sparse_.size();
        try {
            while (page >= sparse_.size()) {
                storage_t storage(
                    utils::construct_at_once, allocator_t<array_t>{ alloc_n_dense_.first() }
                );
                sparse_.emplace_back(std::move(storage));
            }
        }
        catch (...) {
            while (sparse_.size() != current_page) {
                sparse_.pop_back();
            }
        }
    }
    void pop_page_to(const size_type page) noexcept {
        while (sparse_.size() != page) {
            sparse_.pop_back();
        }
    }
    [[nodiscard]] auto contains_impl(const size_type page, const size_type offset) const noexcept
        -> bool {
        // clang-format off
               // if exist this page?
                                       // weather the index could be 0?
                                                       // index equals to 0?       key equals to 0?
        return sparse_.size() > page ? page | offset ? sparse_[page]->at(offset) : alloc_n_dense_.second()[0].first() == 0 : false;
        // clang-format on
    }
    auto erase_without_check_impl(const size_type page, const size_type offset) {
        auto& dense = alloc_n_dense_.second();
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        const size_type index = sparse_[page]->at(offset);
        // UTILS compressed_pair<mapped_type, bool> ret { std::move(alloc_n_dense_.back()), true };
        std::swap(dense[index], dense.back());
        dense.pop_back();
        sparse_[page]->at(offset) = 0;
        // return ret;
    }

    UTILS compressed_pair<allocator_t<value_type>, std::vector<value_type, allocator_t<value_type>>>
        alloc_n_dense_;
    std::vector<storage_t, allocator_t<storage_t>> sparse_;
    std::shared_mutex dense_mutex_;
    std::shared_mutex sparse_mutex_;
};
} // namespace atom
