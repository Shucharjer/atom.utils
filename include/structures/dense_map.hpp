#pragma once
#include "concepts/ranges.hpp"
#if defined(__cpp_concepts)
    #include <concepts>
#endif
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <vector>
#if defined(__cpp_concepts)
    #include "concepts/type.hpp"
#endif
#include "core.hpp"
#include "core/langdef.hpp"
#include "core/pair.hpp"
#include "memory/allocator.hpp"
#include "memory/storage.hpp"
#include "structures.hpp"
#include "thread.hpp"
#include "thread/lock_keeper.hpp"

namespace atom::utils {

#if _HAS_CXX20
template <std::unsigned_integral Key, typename Val, typename Alloc, std::size_t PageSize>
#elif _HAS_CXX17
template <typename Kty, typename Ty, typename Alloc, std::size_t PageSize, typename>
#endif
class dense_map {
    template <typename Target>
    using allocator_t = typename rebind_allocator<Alloc>::template to<Target>::type;

    using alty        = allocator_t<std::pair<Key, Val>>;
    using alty_traits = std::allocator_traits<alty>;

public:
    using key_type    = Key;
    using mapped_type = Val;
    using value_type  = std::pair<key_type, mapped_type>;

    using size_type       = typename alty_traits::size_type;
    using difference_type = typename alty_traits::difference_type;

    /* caused by the graduation design is coming soon, there is no time to fit objects in ecs
     * completely.
     */
    // using allocator_type = allocator_t<value_type>;

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
    _CONSTEXPR20 dense_map() : dense_(), sparse_() {}

    /**
     * @brief Construct with allocator.
     *
     */
    template <typename Al>
    _CONSTEXPR20 dense_map(const Al& allocator) : dense_(allocator), sparse_(allocator) {}

    template <typename Al>
    _CONSTEXPR20 dense_map(std::allocator_arg_t, const Al& al) : dense_(al), sparse_(al) {}

    /**
     * @brief Construct by iterators and allocator.
     *
     */
    template <typename IFirst, typename ILast, typename Al>
    requires concepts::constructible_from_iterator<IFirst, value_type>
    _CONSTEXPR20 dense_map(IFirst first, ILast last, const Al& al) noexcept(
        noexcept(dense_map(std::allocator_arg, al, first, last)))
        : dense_(first, last, al), sparse_(al) {
        for (const auto& [key, val] : dense_) {
            auto page   = page_of(key);
            auto offset = offset_of(key);
            check_page(page);
            sparse_[page]->at(offset) = dense_.size();
        }
    }

    /**
     * @brief Construct by iterators and allocator.
     *
     */
    template <typename IFirst, typename ILast, typename Al>
    requires concepts::constructible_from_iterator<IFirst, value_type>
    _CONSTEXPR20 explicit dense_map(std::allocator_arg_t, const Al& al, IFirst first, ILast last)
        : dense_map(first, last, al) {}

    /**
     * @brief Construct by iterators.
     *
     */
    template <typename IFirst, typename ILast>
    requires concepts::constructible_from_iterator<IFirst, value_type>
    _CONSTEXPR20 dense_map(IFirst first, ILast last) : dense_map(first, last, alty{}) {}

    /**
     * @brief Construct by initializer list and allocator.
     *
     */
    template <typename Al, typename Pair = value_type>
    requires requires {
        typename Pair::first_type;
        typename Pair::second_type;
    } && std::is_constructible_v<value_type, typename Pair::first_type, typename Pair::second_type>
    _CONSTEXPR20 dense_map(std::initializer_list<Pair> il, const Al& allocator = Alloc{})
        : dense_(il.begin(), il.end(), allocator), sparse_(allocator) {
        for (auto i = 0; i < il.size(); ++i) {
            const auto& [key, val] = dense_[i];
            auto page              = page_of(key);
            auto offset            = offset_of(key);
            check_page(page);
            sparse_[page]->at(offset) = i;
        }
    }

    template <typename Al, typename Pair = value_type>
    requires requires {
        typename Pair::first_type;
        typename Pair::second_type;
    } && std::is_constructible_v<value_type, typename Pair::first_type, typename Pair::second_type>
    _CONSTEXPR20 dense_map(std::allocator_arg_t, const Al& al, std::initializer_list<Pair> il)
        : dense_map(il, al) {}

    _CONSTEXPR20 dense_map(dense_map&& that) noexcept
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

    dense_map(const dense_map& that)
        : dense_(that.dense_), sparse_(that.sparse_.size(), that.get_allocator()) {
        for (const auto& page : that.sparse_) {
            unique_storage<array_t, allocator_t<array_t>> storage{ std::allocator_arg,
                                                                   dense_.get_allocator() };
            storage = *page;
            sparse_.emplace_back(std::move(storage));
        }
    }

    dense_map& operator=(const dense_map& that) {
        if (this != &that) {
            dense_map temp(that);
            std::swap(dense_, temp.dense_);
            std::swap(sparse_, temp.sparse_);
        }
        return *this;
    }

    ~dense_map() noexcept = default;

    _NODISCARD auto at(const key_type key) -> Val& {
        auto page   = page_of(key);
        auto offset = offset_of(key);
        shared_lock_keeper lock{ dense_mutex_, sparse_mutex_ };
        return dense_[sparse_[page]->at(offset)].second;
    }

    _NODISCARD auto at(const key_type key) const -> const Val& {
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

    // template <typename... Mappedty>
    // std::pair<iterator, bool> try_emplace(const key_type keyval, Mappedty&&... mapval) {}

    template <typename ValTy>
    _CONSTEXPR26 void emplace(const key_type& key, ValTy&& val) {
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

    template <typename... Tys>
    requires std::is_constructible_v<value_type, Tys...>
    _CONSTEXPR26 std::pair<iterator, bool> emplace(Tys&&... vals) {
        // TODO: finish this function
    }

    auto erase(const key_type key) {
        auto page   = page_of(key);
        auto offset = offset_of(key);

        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        if (contains_impl(key, page, offset)) {
            erase_without_check_impl_unlocked(page, offset);
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
            return dense_.begin() + sparse_[page]->at(offset);
        }
        else {
            return dense_.end();
        }
    }

    _NODISCARD _CONSTEXPR20 bool empty() const noexcept { return dense_.empty(); }

    _NODISCARD _CONSTEXPR20 size_type size() const noexcept { return dense_.size(); }

    void clear() noexcept {
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        sparse_.clear();
        dense_.clear();
    }

    _NODISCARD _CONSTEXPR20 auto front() -> value_type& { return dense_.front(); }
    _NODISCARD _CONSTEXPR20 auto front() const -> const value_type& { return dense_.front(); }

    _NODISCARD _CONSTEXPR20 auto back() -> value_type& { return dense_.back(); }
    _NODISCARD _CONSTEXPR20 auto back() const -> const value_type& { return dense_.back(); }

    _NODISCARD _CONSTEXPR20 auto begin() noexcept -> iterator { return dense_.begin(); }
    _NODISCARD _CONSTEXPR20 auto begin() const noexcept -> const_iterator { return dense_.begin(); }
    _NODISCARD _CONSTEXPR20 auto cbegin() const noexcept -> const_iterator {
        return dense_.cbegin();
    }

    _NODISCARD _CONSTEXPR20 auto end() noexcept -> iterator { return dense_.end(); }
    _NODISCARD _CONSTEXPR20 auto end() const noexcept -> const_iterator { return dense_.end(); }
    _NODISCARD _CONSTEXPR20 auto cend() const noexcept -> const_iterator { return dense_.cend(); }

    _NODISCARD _CONSTEXPR20 auto rbegin() noexcept -> reverse_iterator { return dense_.rbegin(); }
    _NODISCARD _CONSTEXPR20 auto rbegin() const noexcept -> const_reverse_iterator {
        return dense_.rbegin();
    }
    _NODISCARD _CONSTEXPR20 auto crbegin() const noexcept -> const_reverse_iterator {
        return dense_.crbegin();
    }

    _NODISCARD _CONSTEXPR20 auto rend() noexcept -> reverse_iterator { return dense_.rend(); }
    _NODISCARD _CONSTEXPR20 auto rend() const noexcept -> const_reverse_iterator {
        return dense_.rend();
    }
    _NODISCARD _CONSTEXPR20 auto crend() const noexcept -> const_reverse_iterator {
        return dense_.crend();
    }

    _NODISCARD _CONSTEXPR20 auto get_allocator() const noexcept { return dense_.get_allocator(); }

private:
    static size_type page_of(const key_type key) noexcept { return key / PageSize; }
    static size_type offset_of(const key_type key) noexcept { return key % PageSize; }
    void check_page(const size_type page) noexcept {
        const auto current_page = sparse_.size();
        try {
            while (page >= sparse_.size()) {
                // BUG: could not emplace it directly
                storage_t storage{ std::allocator_arg, dense_.get_allocator(), construct_at_once };
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
        return dense_.size() && sparse_.size() > page
                   ? sparse_[page]->at(offset) ? true
                                               : dense_[sparse_[page]->at(offset)].first == key
                   : false;
    }

    /**
     * @brief Erase element without check.
     * May cause error, but faster.
     * We could check before calling this.
     */
    auto erase_without_check_impl(const size_type page, const size_type offset) {
        unique_lock_keeper keeper{ dense_mutex_, sparse_mutex_ };
        erase_without_check_impl_unlocked(page, offset);
    }

    auto erase_without_check_impl_unlocked(const size_type page, const size_type offset) {
        auto& index                                             = sparse_[page]->at(offset);
        auto& back                                              = dense_.back();
        sparse_[page_of(back.first)]->at(offset_of(back.first)) = index;
        std::swap(dense_[index], back);
        dense_.pop_back();
        index = 0;
    }

    std::vector<value_type, allocator_t<value_type>> dense_;
    std::vector<storage_t, allocator_t<storage_t>> sparse_;
    std::shared_mutex dense_mutex_;
    std::shared_mutex sparse_mutex_;
};

#if _HAS_CXX17
template <
    typename Kty, typename Ty, typename Alloc = std::allocator<std::pair<Kty, Ty>>,
    size_t PageSize = k_default_page_size, typename = std::enable_if_t<std::is_unsigned_v<Kty>>>
dense_map(std::initializer_list<std::pair<Kty, Ty>>, Alloc = Alloc())
    -> dense_map<Kty, Ty, Alloc, PageSize>;
template <
    typename Kty, typename Ty, typename Alloc = std::allocator<std::pair<Kty, Ty>>,
    size_t PageSize = k_default_page_size, typename = std::enable_if_t<std::is_unsigned_v<Kty>>>
dense_map(std::initializer_list<std::pair<const Kty, Ty>>, Alloc = Alloc())
    -> dense_map<Kty, Ty, Alloc, PageSize>;
template <
    typename Kty, typename Ty, typename Alloc = std::allocator<std::pair<Kty, Ty>>,
    size_t PageSize = k_default_page_size, typename = std::enable_if_t<std::is_unsigned_v<Kty>>>
dense_map(std::initializer_list<compressed_pair<Kty, Ty>>, Alloc = Alloc())
    -> dense_map<Kty, Ty, Alloc, PageSize>;
template <
    typename Kty, typename Ty, typename Alloc = std::allocator<std::pair<Kty, Ty>>,
    size_t PageSize = k_default_page_size, typename = std::enable_if_t<std::is_unsigned_v<Kty>>>
dense_map(std::initializer_list<compressed_pair<const Kty, Ty>>, Alloc = Alloc())
    -> dense_map<Kty, Ty, Alloc, PageSize>;
#endif

} // namespace atom::utils
