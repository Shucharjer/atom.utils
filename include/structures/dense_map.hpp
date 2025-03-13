#pragma once
#include <concepts>
#include <initializer_list>
#include <vector>
#include "core.hpp"
#include "core/pair.hpp"
#include "memory/allocator.hpp"
#include "memory/storage.hpp"
#include "structures.hpp"
#include "thread.hpp"
#include "thread/lock_keeper.hpp"

namespace atom::utils {

template <
    std::unsigned_integral Key, typename Val, ::atom::utils::concepts::rebindable_allocator Alloc,
    std::size_t PageSize>
class dense_map {
    template <typename Target>
    using allocator_t = typename ::atom::utils::rebind_allocator<Alloc>::template to<Target>::type;

public:
    using key_type    = Key;
    using mapped_type = Val;
    using value_type  = std::pair<key_type, mapped_type>;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    using allocator_type = allocator_t<value_type>;

    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;

    using iterator = typename std::vector<value_type, allocator_t<value_type>>::iterator;
    using const_iterator =
        typename std::vector<value_type, allocator_t<value_type>>::const_iterator;
    using reverse_iterator =
        typename std::vector<value_type, allocator_t<value_type>>::reverse_iterator;
    using const_reverse_iterator =
        typename std::vector<value_type, allocator_t<value_type>>::const_reverse_iterator;

private:
    using array_t            = std::array<size_type, PageSize>;
    using storage_t          = ::atom::utils::unique_storage<array_t, allocator_t<array_t>>;
    using shared_lock_keeper = ::atom::utils::lock_keeper<2, std::shared_mutex, std::shared_lock>;
    using unique_lock_keeper = ::atom::utils::lock_keeper<2, std::shared_mutex, std::unique_lock>;

public:
    /**
     * @brief Default constructor.
     *
     */
    dense_map() : alloc_n_dense_(), sparse_() {}

    /**
     * @brief Construct with allocator.
     *
     */
    template <typename Al>
    requires std::is_constructible_v<allocator_t<value_type>, Al> &&
                 std::is_constructible_v<allocator_t<array_t>, Al> &&
                 std::is_constructible_v<allocator_t<storage_t>, Al>
    explicit dense_map(const Al& allocator)
        : alloc_n_dense_(allocator, allocator_t<value_type>{ allocator }),
          sparse_(allocator_t<storage_t>(allocator)) {}

    /**
     * @brief Construct by iterators and allocator.
     *
     */
    template <typename Iter, typename Al>
    explicit dense_map(Iter first, Iter last, Al&& allocator)
        : alloc_n_dense_(
              allocator, std::vector<value_type, allocator_t<value_type>>(first, last, allocator)),
          sparse_(allocator_t<storage_t>(std::forward<Al>(allocator))) {
        for (const auto& [key, val] : alloc_n_dense_.second()) {
            auto page   = page_of(key);
            auto offset = offset_of(key);
            check_page(page);
            sparse_[page]->at(offset) = alloc_n_dense_.second().size();
        }
    }

    /**
     * @brief Construct by iterators.
     *
     */
    template <typename Iter>
    explicit dense_map(Iter first, Iter last)
        // delegate constructor
        : dense_map(first, last, allocator_t<value_type>{}) {}

    /**
     * @brief Construct by initializer list and allocator.
     *
     */
    template <typename Al>
    explicit dense_map(const std::initializer_list<std::pair<Key, Val>>& list, Al&& allocator)
        : alloc_n_dense_(
              allocator, std::vector<value_type, allocator_t<value_type>>(list, allocator)),
          sparse_(allocator_t<storage_t>(std::forward<Al>(allocator))) {
        for (const auto& [key, val] : alloc_n_dense_.second()) {
            auto page   = page_of(key);
            auto offset = offset_of(key);
            check_page(page);
            sparse_[page]->at(offset) = alloc_n_dense_.second().size();
        }
    }

    /**
     * @brief Construct by initializer list and allocator.
     *
     */
    template <typename Al>
    explicit dense_map(std::initializer_list<std::pair<Key, Val>>&& list, Al&& allocator)
        : alloc_n_dense_(
              allocator,
              std::vector<value_type, allocator_t<value_type>>(std::move(list), allocator)),
          sparse_(allocator_t<storage_t>(std::forward<Al>(allocator))) {
        const auto size = alloc_n_dense_.second().size();
        for (auto i = 0; i < size; ++i) {
            const auto& [key, val] = alloc_n_dense_.second()[i];
            const auto page        = page_of(key);
            const auto offset      = offset_of(key);
            check_page(page);
            sparse_[page]->at(offset) = i;
        }
    }

    /**
     * @brief Construct by initializer list.
     *
     */
    explicit dense_map(const std::initializer_list<std::pair<Key, Val>>& list)
        // delegate constructor
        : dense_map(list, allocator_t<value_type>{}) {}

    /**
     * @brief Construct by initializer list.
     *
     */
    explicit dense_map(std::initializer_list<std::pair<Key, Val>>&& list)
        // delegate consturctor
        : dense_map(std::move(list), allocator_t<value_type>{}) {}

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

    dense_map(const dense_map& that)
        : alloc_n_dense_(that.alloc_n_dense_), sparse_(that.sparse_.size()) {
        //
        for (const auto& page : that.sparse_) {
            unique_storage<array_t, allocator_t<array_t>> storage{ alloc_n_dense_.first() };
            storage = *page;
            sparse_.emplace_back(std::move(storage));
        }
    }

    // TODO:
    dense_map& operator=(const dense_map&) = delete;

    ~dense_map() noexcept = default;

    auto at(const key_type key) -> Val& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return alloc_n_dense_.second()[sparse_[page]->at(offset)].second;
    }

    [[nodiscard]] auto at(const key_type key) const -> const Val& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return alloc_n_dense_.second()[sparse_[page]->at(offset)].second;
    }

    auto operator[](const key_type key) -> Val& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return alloc_n_dense_.second()[sparse_[page]->at(offset)].second;
    }

    auto operator[](const key_type key) const -> const Val& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return alloc_n_dense_.second()[sparse_[page]->at(offset)].second;
    }

    template <typename ValTy>
    void emplace(const key_type& key, ValTy&& val) {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        auto current_page_count = sparse_.size();
        try {
            check_page(page);
            sparse_[page]->at(offset) = alloc_n_dense_.second().size();
            try {
                alloc_n_dense_.second().emplace_back(key, std::forward<ValTy>(val));
            }
            catch (...) {
                sparse_[page]->at(offset) = 0;
                throw;
            }
        }
        catch (...) {
            pop_page_to(current_page_count);
        }
    }

    auto erase(const key_type key) {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        shared_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        if (contains_impl(key, page, offset)) {
            keeper.run_away();
            erase_without_check_impl(page, offset);
        }
    }

    auto erase_without_check(const key_type key) {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        erase_without_check_impl(page, offset);
    }

    void reserve(const size_type size) {
        auto page = page_of(size);
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        check_page(page);
        alloc_n_dense_.second().reserve(size);
    }

    auto contains(const key_type key) const -> bool {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        return contains_impl(key, page, offset);
    }

    [[nodiscard]] auto find(const key_type key) noexcept -> iterator {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        shared_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        if (contains_impl(key, page, offset)) {
            return alloc_n_dense_.second().begin() + sparse_[page]->at(offset);
        }
        else {
            return alloc_n_dense_.second().end();
        }
    }

    [[nodiscard]] auto find(const key_type key) const noexcept -> const_iterator {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        shared_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        if (contains_impl(key, page, offset)) {
            return alloc_n_dense_.second().cbegin() + sparse_[page]->at(offset);
        }
        else {
            return alloc_n_dense_.second().cend();
        }
    }

    [[nodiscard]] bool empty() const noexcept { return alloc_n_dense_.second().empty(); }

    [[nodiscard]] size_type size() const noexcept { return alloc_n_dense_.second().size(); }

    void clear() noexcept {
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        sparse_.clear();
        alloc_n_dense_.second().clear();
    }

    auto front() -> mapped_type& { return alloc_n_dense_.second().front(); }
    [[nodiscard]] auto front() const -> const mapped_type& {
        return alloc_n_dense_.second().front();
    }

    auto back() -> mapped_type& { return alloc_n_dense_.second().back(); }
    [[nodiscard]] auto back() const -> const mapped_type& { return alloc_n_dense_.second().back(); }

    auto begin() noexcept -> iterator { return alloc_n_dense_.second().begin(); }
    [[nodiscard]] auto begin() const noexcept -> const_iterator {
        return alloc_n_dense_.second().cbegin();
    }
    [[nodiscard]] auto cbegin() const noexcept -> const_iterator {
        return alloc_n_dense_.second().cbegin();
    }

    auto end() noexcept -> iterator { return alloc_n_dense_.second().end(); }
    [[nodiscard]] auto end() const noexcept -> const_iterator {
        return alloc_n_dense_.second().cend();
    }
    [[nodiscard]] auto cend() const noexcept -> const_iterator {
        return alloc_n_dense_.second().cend();
    }

    auto rbegin() noexcept -> reverse_iterator { return alloc_n_dense_.second().rbegin(); }
    [[nodiscard]] auto rbegin() const noexcept -> const_reverse_iterator {
        return alloc_n_dense_.second().rbegin();
    }
    [[nodiscard]] auto crbegin() const noexcept -> const_reverse_iterator {
        return alloc_n_dense_.second().crbegin();
    }

    auto rend() noexcept -> reverse_iterator { return alloc_n_dense_.second().rend(); }
    [[nodiscard]] auto rend() const noexcept -> const_reverse_iterator {
        return alloc_n_dense_.second().rend();
    }
    [[nodiscard]] auto crend() const noexcept -> const_reverse_iterator {
        return alloc_n_dense_.second().crend();
    }

private:
    static size_type page_of(const key_type key) noexcept { return key / PageSize; }
    static size_type offset_of(const key_type key) noexcept { return key % PageSize; }
    void check_page(const size_type page) {
        const auto current_page = sparse_.size();
        try {
            while (page >= sparse_.size()) {
                storage_t storage(
                    utils::construct_at_once, allocator_t<array_t>{ alloc_n_dense_.first() });
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
    [[nodiscard]] auto contains_impl(
        const key_type key, const size_type page, const size_type offset) const noexcept -> bool {
        auto& dense = alloc_n_dense_.second();
        return dense.size() && sparse_.size() > page
                   ? sparse_[page]->at(offset) ? true
                                               : dense[sparse_[page]->at(offset)].first == key
                   : false;
    }
    auto erase_without_check_impl(const size_type page, const size_type offset) {
        auto& dense = alloc_n_dense_.second();
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        auto& index                                             = sparse_[page]->at(offset);
        auto& back                                              = dense.back();
        sparse_[page_of(back.first)]->at(offset_of(back.first)) = index;
        std::swap(dense[index], back);
        dense.pop_back();
        index = 0;
    }

    compressed_pair<allocator_t<value_type>, std::vector<value_type, allocator_t<value_type>>>
        alloc_n_dense_;
    std::vector<storage_t, allocator_t<storage_t>> sparse_;
    std::shared_mutex dense_mutex_;
    std::shared_mutex sparse_mutex_;
};

template <std::unsigned_integral Key, typename Val, std::size_t PageSize = k_default_page_size>
using sync_dense_map = dense_map<Key, Val, sync_allocator<std::pair<Key, Val>>, PageSize>;

template <std::unsigned_integral Key, typename Val, std::size_t PageSize = k_default_page_size>
using unsync_dense_map = dense_map<Key, Val, unsync_allocator<std::pair<Key, Val>>, PageSize>;

namespace pmr {

template <std::unsigned_integral Key, typename Val, std::size_t PageSize>
class dense_map {
public:
    using key_type    = Key;
    using mapped_type = Val;
    using value_type  = std::pair<key_type, mapped_type>;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    using pointer         = value_type*;
    using const_pointer   = const value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;

    using iterator               = typename std::pmr::vector<value_type>::iterator;
    using const_iterator         = typename std::pmr::vector<value_type>::const_iterator;
    using reverse_iterator       = typename std::pmr::vector<value_type>::reverse_iterator;
    using const_reverse_iterator = typename std::pmr::vector<value_type>::const_reverse_iterator;

private:
    using array_t            = std::array<size_type, PageSize>;
    using storage_t          = ::atom::utils::unique_storage<array_t>;
    using shared_lock_keeper = ::atom::utils::lock_keeper<2, std::shared_mutex, std::shared_lock>;
    using unique_lock_keeper = ::atom::utils::lock_keeper<2, std::shared_mutex, std::unique_lock>;

public:
    /**
     * @brief Default constructor.
     *
     */
    dense_map() : dense_(), sparse_() {}

    /**
     * @brief Construct by iterators.
     *
     */
    template <typename Iter>
    explicit dense_map(Iter first, Iter last) : dense_(first, last), sparse_() {
        for (const auto& [key, val] : dense_) {
            auto page   = page_of(key);
            auto offset = offset_of(key);
            check_page(page);
            sparse_[page]->at(offset) = dense_.size();
        }
    }

    /**
     * @brief Construct by initializer list.
     *
     */
    explicit dense_map(const std::initializer_list<std::pair<Key, Val>>& list)
        : dense_(list), sparse_() {
        for (const auto& [key, val] : dense_) {
            auto page   = page_of(key);
            auto offset = offset_of(key);
            check_page(page);
            sparse_[page]->at(offset) = dense_.size();
        }
    }

    /**
     * @brief Construct by initializer list.
     *
     */
    explicit dense_map(std::initializer_list<std::pair<Key, Val>>&& list)
        // delegate consturctor
        : dense_map(std::move(list)) {}

    dense_map(dense_map&& that) noexcept
        : dense_(std::move(that.dense_)), sparse_(std::move(that.sparse_)) {}

    dense_map& operator=(dense_map&& that) noexcept {
        if (this != &that) {
            unique_lock_keeper that_keeper{ that.dense_mutex_, that.sparse_mutex_ };
            unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
            dense_  = std::move(that.dense_);
            sparse_ = std::move(that.sparse_);
        }
        return *this;
    }

    dense_map(const dense_map& that) : dense_(that.dense_), sparse_(that.sparse_.size()) {
        //
        for (const auto& page : that.sparse_) {
            unique_storage<array_t> storage{};
            storage = *page;
            sparse_.emplace_back(std::move(storage));
        }
    }

    // TODO:
    dense_map& operator=(const dense_map&) = delete;

    ~dense_map() noexcept = default;

    auto at(const key_type key) -> Val& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return dense_[sparse_[page]->at(offset)].second;
    }

    [[nodiscard]] auto at(const key_type key) const -> const Val& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return dense_[sparse_[page]->at(offset)].second;
    }

    auto operator[](const key_type key) -> Val& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return dense_[sparse_[page]->at(offset)].second;
    }

    auto operator[](const key_type key) const -> const Val& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return dense_[sparse_[page]->at(offset)].second;
    }

    template <typename ValTy>
    void emplace(const key_type& key, ValTy&& val) {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        auto current_page_count = sparse_.size();
        try {
            check_page(page);
            sparse_[page]->at(offset) = dense_.size();
            try {
                dense_.emplace_back(key, std::forward<ValTy>(val));
            }
            catch (...) {
                sparse_[page]->at(offset) = 0;
                throw;
            }
        }
        catch (...) {
            pop_page_to(current_page_count);
        }
    }

    auto erase(const key_type key) {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        shared_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        if (contains_impl(key, page, offset)) {
            keeper.run_away();
            erase_without_check_impl(page, offset);
        }
    }

    auto erase_without_check(const key_type key) {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        erase_without_check_impl(page, offset);
    }

    void reserve(const size_type size) {
        auto page = page_of(size);
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        check_page(page);
        dense_.reserve(size);
    }

    auto contains(const key_type key) const -> bool {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        return contains_impl(key, page, offset);
    }

    [[nodiscard]] auto find(const key_type key) noexcept -> iterator {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        shared_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        if (contains_impl(key, page, offset)) {
            return dense_.begin() + sparse_[page]->at(offset);
        }
        else {
            return dense_.end();
        }
    }

    [[nodiscard]] auto find(const key_type key) const noexcept -> const_iterator {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        shared_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        if (contains_impl(key, page, offset)) {
            return dense_.cbegin() + sparse_[page]->at(offset);
        }
        else {
            return dense_.cend();
        }
    }

    [[nodiscard]] bool empty() const noexcept { return dense_.empty(); }

    [[nodiscard]] size_type size() const noexcept { return dense_.size(); }

    void clear() noexcept {
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        sparse_.clear();
        dense_.clear();
    }

    auto front() -> mapped_type& { return dense_.front(); }
    [[nodiscard]] auto front() const -> const mapped_type& { return dense_.front(); }

    auto back() -> mapped_type& { return dense_.back(); }
    [[nodiscard]] auto back() const -> const mapped_type& { return dense_.back(); }

    auto begin() noexcept -> iterator { return dense_.begin(); }
    [[nodiscard]] auto begin() const noexcept -> const_iterator { return dense_.cbegin(); }
    [[nodiscard]] auto cbegin() const noexcept -> const_iterator { return dense_.cbegin(); }

    auto end() noexcept -> iterator { return dense_.end(); }
    [[nodiscard]] auto end() const noexcept -> const_iterator { return dense_.cend(); }
    [[nodiscard]] auto cend() const noexcept -> const_iterator { return dense_.cend(); }

    auto rbegin() noexcept -> reverse_iterator { return dense_.rbegin(); }
    [[nodiscard]] auto rbegin() const noexcept -> const_reverse_iterator { return dense_.rbegin(); }
    [[nodiscard]] auto crbegin() const noexcept -> const_reverse_iterator {
        return dense_.crbegin();
    }

    auto rend() noexcept -> reverse_iterator { return dense_.rend(); }
    [[nodiscard]] auto rend() const noexcept -> const_reverse_iterator { return dense_.rend(); }
    [[nodiscard]] auto crend() const noexcept -> const_reverse_iterator { return dense_.crend(); }

private:
    static size_type page_of(const key_type key) noexcept { return key / PageSize; }
    static size_type offset_of(const key_type key) noexcept { return key % PageSize; }
    void check_page(const size_type page) {
        const auto current_page = sparse_.size();
        try {
            while (page >= sparse_.size()) {
                storage_t storage(utils::construct_at_once);
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
    [[nodiscard]] auto contains_impl(
        const key_type key, const size_type page, const size_type offset) const noexcept -> bool {
        return dense_.size() && (sparse_.size() > page)
                   ? sparse_[page]->at(offset) ? true
                                               : dense_[sparse_[page]->at(offset)].first == key
                   : false;
    }
    auto erase_without_check_impl(const size_type page, const size_type offset) {
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        auto& index                                             = sparse_[page]->at(offset);
        auto& back                                              = dense_.back();
        sparse_[page_of(back.first)]->at(offset_of(back.first)) = index;
        std::swap(dense_[index], back);
        dense_.pop_back();
        index = 0;
        // return ret;
    }

    std::pmr::vector<value_type> dense_;
    std::pmr::vector<storage_t> sparse_;
    std::shared_mutex dense_mutex_;
    std::shared_mutex sparse_mutex_;
};

} // namespace pmr

} // namespace atom::utils
