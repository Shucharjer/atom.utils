#pragma once
#include <concepts>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <vector>
#include "concepts/type.hpp"
#include "core.hpp"
#include "core/langdef.hpp"
#include "core/pair.hpp"
#include "memory/allocator.hpp"
#include "memory/storage.hpp"
#include "structures.hpp"
#include "thread.hpp"
#include "thread/lock_keeper.hpp"

namespace atom::utils {

template <std::unsigned_integral Key, typename Val, typename Alloc, std::size_t PageSize>
class dense_map {
    template <typename Target>
    using allocator_t = typename ::atom::utils::rebind_allocator<Alloc>::template to<Target>::type;

    using alty        = allocator_t<compressed_pair<Key, Val>>;
    using alty_traits = std::allocator_traits<alty>;

public:
    using key_type    = Key;
    using mapped_type = Val;
    using value_type  = compressed_pair<key_type, mapped_type>;

    using size_type       = typename alty_traits::size_type;
    using difference_type = typename alty_traits::difference_type;

    using allocator_type = allocator_t<value_type>;

    using pointer         = typename alty_traits::pointer;
    using const_pointer   = typename alty_traits::const_pointer;
    using reference       = value_type&;
    using const_reference = const value_type&;

private:
    using vector = std::vector<value_type, allocator_t<value_type>>;

public:
    using iterator               = typename vector::iterator;
    using const_iterator         = typename vector::const_iterator;
    using reverse_iterator       = typename vector::reverse_iterator;
    using const_reverse_iterator = typename vector::const_reverse_iterator;

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
    _CONSTEXPR20 dense_map() noexcept(std::is_nothrow_default_constructible_v<alty>)
        : alloc_n_dense_(), sparse_() {}

    /**
     * @brief Construct with allocator.
     *
     */
    template <typename Al>
    _CONSTEXPR20 explicit dense_map(const Al& allocator) noexcept
        : alloc_n_dense_(allocator, alty{ allocator }),
          sparse_(allocator_t<storage_t>{ allocator }) {}

    /**
     * @brief Construct by iterators and allocator.
     *
     */
    template <typename Iter, typename Al>
    explicit dense_map(std::allocator_arg_t, const Al& allocator, Iter first, Iter last)
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
        : dense_map(std::allocator_arg, alty{}, first, last) {}

    /**
     * @brief Construct by initializer list and allocator.
     *
     */
    template <typename Al, concepts::pair Pair>
    explicit dense_map(const std::initializer_list<Pair>& list, Al&& allocator)
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
    template <typename Al, concepts::pair Pair>
    explicit dense_map(std::initializer_list<Pair>&& list, Al&& allocator)
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
    template <concepts::pair Pair>
    explicit dense_map(const std::initializer_list<Pair>& list)
        // delegate constructor
        : dense_map(list, allocator_t<value_type>{}) {}

    /**
     * @brief Construct by initializer list.
     *
     */
    template <concepts::pair Pair>
    explicit dense_map(std::initializer_list<Pair>&& list)
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
        : alloc_n_dense_(that.alloc_n_dense_.first(), that.alloc_n_dense_.second()),
          sparse_(that.sparse_.size()) {
        for (const auto& page : that.sparse_) {
            unique_storage<array_t, allocator_t<array_t>> storage{ alloc_n_dense_.first() };
            storage = *page;
            sparse_.emplace_back(std::move(storage));
        }
    }

    dense_map& operator=(const dense_map& that) {
        if (this != &that) {
            dense_map temp(that);
            std::swap(alloc_n_dense_, temp.alloc_n_dense_);
            std::swap(sparse_, temp.sparse_);
        }
        return *this;
    }

    ~dense_map() noexcept = default;

    _NODISCARD auto at(const key_type key) -> Val& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return alloc_n_dense_.second()[sparse_[page]->at(offset)].second;
    }

    _NODISCARD auto at(const key_type key) const -> const Val& {
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
            return alloc_n_dense_.second().begin() + sparse_[page]->at(offset);
        }
        else {
            return alloc_n_dense_.second().end();
        }
    }

    _NODISCARD _CONSTEXPR20 bool empty() const noexcept { return alloc_n_dense_.second().empty(); }

    _NODISCARD _CONSTEXPR20 size_type size() const noexcept {
        return alloc_n_dense_.second().size();
    }

    void clear() noexcept {
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        sparse_.clear();
        alloc_n_dense_.second().clear();
    }

    _NODISCARD _CONSTEXPR20 auto front() -> value_type& { return alloc_n_dense_.second().front(); }
    _NODISCARD _CONSTEXPR20 auto front() const -> const value_type& {
        return alloc_n_dense_.second().front();
    }

    _NODISCARD _CONSTEXPR20 auto back() -> value_type& { return alloc_n_dense_.second().back(); }
    _NODISCARD _CONSTEXPR20 auto back() const -> const value_type& {
        return alloc_n_dense_.second().back();
    }

    _NODISCARD _CONSTEXPR20 auto begin() noexcept -> iterator {
        return alloc_n_dense_.second().begin();
    }
    _NODISCARD _CONSTEXPR20 auto begin() const noexcept -> const_iterator {
        return alloc_n_dense_.second().begin();
    }
    _NODISCARD _CONSTEXPR20 auto cbegin() const noexcept -> const_iterator {
        return alloc_n_dense_.second().cbegin();
    }

    _NODISCARD _CONSTEXPR20 auto end() noexcept -> iterator {
        return alloc_n_dense_.second().end();
    }
    _NODISCARD _CONSTEXPR20 auto end() const noexcept -> const_iterator {
        return alloc_n_dense_.second().end();
    }
    _NODISCARD _CONSTEXPR20 auto cend() const noexcept -> const_iterator {
        return alloc_n_dense_.second().cend();
    }

    _NODISCARD _CONSTEXPR20 auto rbegin() noexcept -> reverse_iterator {
        return alloc_n_dense_.second().rbegin();
    }
    _NODISCARD _CONSTEXPR20 auto rbegin() const noexcept -> const_reverse_iterator {
        return alloc_n_dense_.second().rbegin();
    }
    _NODISCARD _CONSTEXPR20 auto crbegin() const noexcept -> const_reverse_iterator {
        return alloc_n_dense_.second().crbegin();
    }

    _NODISCARD _CONSTEXPR20 auto rend() noexcept -> reverse_iterator {
        return alloc_n_dense_.second().rend();
    }
    _NODISCARD _CONSTEXPR20 auto rend() const noexcept -> const_reverse_iterator {
        return alloc_n_dense_.second().rend();
    }
    _NODISCARD _CONSTEXPR20 auto crend() const noexcept -> const_reverse_iterator {
        return alloc_n_dense_.second().crend();
    }

    _NODISCARD _CONSTEXPR20 auto& get_allocator() const noexcept { return alloc_n_dense_.first(); }

private:
    static size_type page_of(const key_type key) noexcept { return key / PageSize; }
    static size_type offset_of(const key_type key) noexcept { return key % PageSize; }
    void check_page(const size_type page) noexcept {
        const auto current_page = sparse_.size();
        try {
            while (page >= sparse_.size()) {
                storage_t storage(
                    utils::construct_at_once, allocator_t<array_t>{ alloc_n_dense_.first() });
                sparse_.emplace_back(
                    std::allocator_arg, alloc_n_dense_.first(), std::move(storage));
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

    /**
     * @brief Erase element without check.
     * May cause error, but faster.
     * We could check before calling this.
     */
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

    compressed_pair<allocator_t<value_type>, vector> alloc_n_dense_;
    std::vector<storage_t, allocator_t<storage_t>> sparse_;
    std::shared_mutex dense_mutex_;
    std::shared_mutex sparse_mutex_;
};

} // namespace atom::utils
