#pragma once
#include <array>
#include <concepts>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <type_traits>
#include <vector>
#include "sparse.h"
#include "type_traits.h"

namespace atom::utils {
// Key是整型，Val的类型不一定是基本类型
template <std::integral Key, typename Val>
struct density_node;

template <std::integral Key, typename Val>
struct density_node {
    using self_type   = density_node;
    using key_type    = Key;
    using mapped_type = Val;

    key_type first{};
    mapped_type second{};

    explicit density_node(
        key_type key_ = key_type{}
    ) noexcept(std::is_nothrow_default_constructible_v<mapped_type>)
        : first{ key_ } {}

    // explicit density_node(
    //     key_type first, mapped_type& second
    // ) noexcept(std::is_nothrow_copy_constructible_v<mapped_type>)
    //     : first(first), second(second) {}

    // explicit density_node(
    //     key_type first, mapped_type&& second
    // ) noexcept(std::is_nothrow_move_constructible_v<mapped_type>)
    //     : first(first), second(std::move(second)) {}

    template <typename Mapped>
    explicit density_node(key_type first, Mapped&& second)
        : first(first), second(std::forward<Mapped>(second)) {}

    density_node(density_node const& obj) noexcept(std::is_nothrow_copy_constructible_v<Val>)
        : first(obj.first), second(obj.second) {}

    density_node(density_node&& obj) noexcept(std::is_nothrow_move_constructible_v<Val>)
        : first(obj.first), second(std::move(obj.second)) {}

    auto operator=(density_node const& obj) noexcept(std::is_nothrow_copy_assignable_v<Val>)
        -> density_node& {
        if (this == std::addressof(obj)) {
            return *this;
        }
        first = obj.first;
        Val val(obj.second);
        second = std::move(val);
        return *this;
    }

    auto operator=(density_node&& obj) noexcept(std::is_nothrow_move_assignable_v<Val>)
        -> density_node& {
        if (this == std::addressof(obj)) {
            return *this;
        }

        first  = obj.first;
        second = std::move(obj.second);

        return *this;
    }
    ~density_node() noexcept = default;
    auto operator==(density_node const& obj) const -> bool {
        return first == obj.first && second == obj.second;
    }
    auto operator!=(density_node const& obj) const -> bool {
        return first != obj.first || second != obj.second;
    }
    [[nodiscard]] auto is_invaild() const noexcept -> bool {
        return first == static_cast<key_type>(-1);
    }
};

template <typename Key, typename Val>
requires std::is_integral_v<Key>
struct density_node<Key, Val*> {
    using self_type   = density_node;
    using key_type    = Key;
    using mapped_type = Val*;

    key_type first{};
    mapped_type second{};

    explicit density_node(key_type first = {}) noexcept : first{ first } {}
    explicit density_node(key_type first, mapped_type second) noexcept
        : first{ first }, second{ second } {}

    ~density_node() = default;

    density_node(density_node const& obj) noexcept : first{ obj.first }, second{ obj.second } {}
    density_node(density_node&& obj) noexcept : first{ obj.first }, second{ obj.second } {
        obj.second = nullptr;
    }

    auto operator=(density_node const& obj) noexcept -> density_node& {
        // 整型和指针的拷贝代价较小，不进行判断会有略微的性能提升
        // if (this == &obj) return *this;
        first  = obj.first;
        second = obj.second;
        return *this;
    }
    auto operator=(density_node&& obj) noexcept -> density_node& {
        // 整型和指针的拷贝代价较小，不进行判断会有略微的性能提升
        // if (this == &obj) return *this;
        first      = obj.first;
        second     = obj.second;
        obj.second = nullptr;
        return *this;
    }

    auto operator==(density_node const& obj) const noexcept -> bool {
        return first == obj.first && second == obj.second;
    }
    auto operator!=(density_node const& obj) const noexcept -> bool {
        return first != obj.first && second != obj.second;
    }
    [[nodiscard]] auto is_invaild() const noexcept -> bool {
        return first == static_cast<key_type>(-1);
    }
};

template <std::integral Key, typename Val, size_t PageSize, std::unsigned_integral IndexType>
// Key must be an integral type
// make sure PageSize is a positive value
// IndexType must be an unsigned integral type
requires is_positive_integral_v<PageSize>
class sparse_map {
    [[nodiscard]] constexpr auto page_(const Key key) const noexcept -> IndexType {
        return key / PageSize;
    }
    [[nodiscard]] constexpr auto offset_(const Key key) const noexcept -> IndexType {
        return key % PageSize;
    }

private:
    std::vector<density_node<Key, Val>> density_;
    // the index of an element could be litter than zero, so we set the type of array element
    // std::size_t
    std::vector<std::unique_ptr<std::array<IndexType, PageSize>>> sparses_;
    density_node<Key, Val> _invalid_node;

    IndexType page_count_;
    IndexType count_;

    [[nodiscard]] inline auto contains_(const Key key, const IndexType page, const IndexType offset)
        const noexcept -> bool {
        if (!count_ || page >= page_count_) {
            return false;
        }

        switch (sparses_[page]->at(offset)) {
        case 0:
            return density_[0].first == key;
        default:
            return true;
        }
    }

    inline void check_page_count_(const IndexType page) {
        auto page_count = page_count_;
        try {
            // when page_count_ larger than page, the capacity is enough
            while (page_count <= page) {
                sparses_.push_back(std::make_unique<std::array<IndexType, PageSize>>());
                ++page_count;
            }
        }
        catch (...) {
            while (page_count != page_count_) {
                sparses_.pop_back();
            }
            throw;
        }
        page_count_ = page_count;
    }

public:
    using self_type      = sparse_map;
    using key_type       = Key;
    using mapped_type    = Val;
    using value_type     = density_node<Key, Val>;
    using iterator       = typename std::vector<density_node<Key, Val>>::iterator;
    using const_iterator = typename std::vector<density_node<key_type, mapped_type>>::const_iterator;

    sparse_map() : page_count_(1), count_(0), _invalid_node(static_cast<Key>(-1)) {
        sparses_.emplace_back(std::make_unique<std::array<IndexType, PageSize>>());
    }

    ~sparse_map() = default;

    sparse_map(const sparse_map& obj)
        : page_count_(obj.page_count_), count_(obj.count_), density_(obj.density_) {
        // for (int i = 0; i < page_count_; i++) {
        //     sparses_.emplace_back(std::make_unique<std::array<std::size_t, PageSize>>());
        //     (*sparses_[i]) = (*obj.sparses_[i]);
        // }

        if (density_.size()) {
            try {
                sparses_ = std::vector<std::unique_ptr<std::array<std::size_t, PageSize>>>(
                    page_count_, std::make_unique<std::array<std::size_t, PageSize>>()
                );

                for (std::size_t i = 0; i < density_.size(); ++i) {
                    auto& node                                           = density_[i];
                    sparses_[page_(node.first)]->at(offset_(node.first)) = i;
                }
            }
            catch (...) {
                page_count_ = 0;
                count_      = 0;
                density_.clear();
            }
        }
    }

    sparse_map(sparse_map&& obj
    ) noexcept(std::is_nothrow_move_constructible_v<std::vector<density_node<Key, Val>>> && std::is_nothrow_move_constructible_v<std::vector<std::unique_ptr<std::array<IndexType, PageSize>>>>)
        : page_count_(obj.page_count_), count_(obj.count_), density_(std::move(obj.density_)),
          sparses_(std::move(obj.sparses_)) {}

    auto operator=(const sparse_map& obj) -> sparse_map& {
        if (this == &obj) {
            return *this;
        }

        try {
            sparse_map temp(obj);
            *this = std::move(temp);
        }
        catch (...) {
            throw;
        }

        return *this;
    }
    auto operator=(sparse_map&& obj
    ) noexcept(std::is_nothrow_move_constructible_v<std::vector<density_node<Key, Val>>> && std::is_nothrow_move_constructible_v<std::vector<std::unique_ptr<std::array<IndexType, PageSize>>>>)
        -> sparse_map& {
        if (this == &obj) {
            return *this;
        }

        density_    = std::move(obj.density_);
        sparses_    = std::move(obj.sparses_);
        page_count_ = obj.page_count_;
        count_      = obj.count_;

        return *this;
    }

    /**
     * @brief
     *
     * @tparam Valty
     * @param key
     * @param val
     * @return requires
     */
    template <typename Valty>
    void emplace(const key_type key, Valty&& val) {
        IndexType page   = page_(key);
        IndexType offset = offset_(key);

        if (contains_(key, page, offset)) {
            return;
        }

        try {
            density_.emplace_back(key, std::forward<Valty>(val));

            try {
                check_page_count_(page);
                sparses_[page]->at(offset) = count_++;
            }
            catch (...) {
                density_.pop_back();
            }
        }
        catch (...) {
        }
    }

    /**
     * @brief
     *
     * @tparam Valty
     * @param key
     * @param val
     * @return requires
     */
    template <typename Valty>
    void set(const key_type key, Valty&& val) noexcept {
        IndexType page   = page_(key);
        IndexType offset = offset_(key);

        if (!contains_(key, page, offset)) {
            return;
        }

        IndexType& index    = sparses_[page]->at(offset);
        density_[index].val = std::forward<Valty>(val);
    }

    auto erase(const_iterator Where) noexcept -> iterator { density_.erase(Where); }

    /**
     * @brief Erase the element pair which key equals to the param
     *
     * @param key Key
     */
    void erase(const key_type key) noexcept {
        IndexType page   = page_(key);
        IndexType offset = offset_(key);

        if (!contains_(key, page, offset)) {
            return;
        }

        IndexType& index_remove                         = sparses_[page]->at(offset);
        density_node<key_type, mapped_type>& final_node = density_.back();
        IndexType& index_final = sparses_[page_(final_node.first)]->at(offset_(final_node.first));
        if (index_remove != index_final) {
            // std::swap(density_[index_remove], final_node);
            // std::swap(index_remove, index_final);
            density_[index_remove] = std::move(final_node);
            index_final            = index_remove;
        }
        index_remove = 0;
        density_.pop_back();

        --count_;
    }

    /**
     * @brief
     *
     * @param key
     * @return true
     * @return false
     */
    [[nodiscard]] auto contains(const key_type key) const noexcept -> bool {
        IndexType page   = page_(key);
        IndexType offset = offset_(key);

        return contains_(key, page, offset);
    }

    /**
     * @brief
     *
     * @param key
     * @return auto
     */
    [[nodiscard]] auto find(const key_type key) noexcept {
        IndexType page   = page_(key);
        IndexType offset = offset_(key);

        return page < page_count_ && (sparses_[page]->at(offset) ||
                                      (!density_.empty() && density_[0].first == key))
                   ? density_.begin() +
                         (&(density_[sparses_[page]->at(offset)]) - &(*density_.begin()))
                   : density_.end();
    }

    /**
     * @brief
     *
     * @param key
     * @return auto
     */
    [[nodiscard]] auto find(const key_type key) const noexcept {
        IndexType page   = page_(key);
        IndexType offset = offset_(key);

        return page < page_count_ && (sparses_[page]->at(offset) ||
                                      (!density_.empty() && density_[0].first == key))
                   ? density_.cbegin() +
                         (&(density_[sparses_[page]->at(offset)]) - &(*density_.cbegin()))
                   : density_.cend();
    }

    /**
     * @brief
     *
     * @param key
     * @return mapped_type&
     */
    [[nodiscard]] mapped_type& at(const key_type key) noexcept {
        IndexType page   = page_(key);
        IndexType offset = offset_(key);

        if (!contains_(key, page, offset)) {
            return _invalid_node.second;
        }

        return density_[sparses_[page]->at(offset)].second;
    }

    /**
     * @brief
     *
     * @param key
     * @return const mapped_type&
     */
    [[nodiscard]] const mapped_type& at(const key_type key) const noexcept {
        IndexType page   = page_(key);
        IndexType offset = offset_(key);

        if (!contains_(key, page, offset)) {
            return _invalid_node.val;
        }

        return density_[sparses_[page]->at(offset)].val;
    }

    /**
     * @brief
     *
     * @param key
     * @return std::optional<IndexType>
     */
    [[nodiscard]] std::optional<IndexType> index_of(const key_type key) const noexcept {
        IndexType page   = page_(key);
        IndexType offset = offset_(key);

        if (!contains_(key, page, offset)) {
            return std::optional<IndexType>();
        }

        return std::optional<IndexType>(sparses_[page]->at(offset));
    }

    /**
     * @brief
     *
     * @return true When map not empty
     * @return false Otherwise
     */
    [[nodiscard]] inline bool empty() const noexcept { return !count_; }

    /**
     * @brief Get the count of element pairs
     *
     * @return IndexType Count
     */
    [[nodiscard]] inline IndexType size() const noexcept { return count_; }

    /**
     * @brief Clear all element pairs
     *
     */
    inline void clear() noexcept {
        density_.clear();
        sparses_.clear();
        page_count_ = 0;
        count_      = 0;
    }

    /**
     * @brief If key exists, return its mapped value, otherwise create one
     *
     * @param key
     * @return mapped_type&
     */
    [[nodiscard]] mapped_type& operator[](const key_type key) {
        auto page   = page_(key);
        auto offset = offset_(key);

        if (!contains_(key, page, offset)) {
            try {
                density_.push_back(density_node<key_type, mapped_type>(key, mapped_type()));
                ++count_;

                try {
                    check_page_count_(page);
                    sparses_[page]->at(offset) = count_;
                }
                catch (...) {
                    density_.pop_back();
                    --count_;
                }
            }
            catch (...) {
                throw;
            }
        }

        return density_[sparses_[page]->at(offset)].second;
    }

    auto raw() noexcept { return density_; }

    inline auto front() noexcept { return density_.front(); }
    inline auto back() noexcept { return density_.back(); }

    inline auto begin() noexcept { return density_.begin(); }
    [[nodiscard]] inline auto begin() const noexcept { return density_.cbegin(); }
    [[nodiscard]] inline auto cbegin() const noexcept { return density_.cbegin(); }
    inline auto rbegin() noexcept { return density_.rbegin(); }
    [[nodiscard]] inline auto crbegin() const noexcept { return density_.crbegin(); }
    inline auto end() noexcept { return density_.end(); }
    [[nodiscard]] inline auto end() const noexcept { return density_.cend(); }
    [[nodiscard]] inline auto cend() const noexcept { return density_.cend(); }
    inline auto rend() noexcept { return density_.rend(); }
    [[nodiscard]] inline auto crend() const noexcept { return density_.crend(); }
};

/**
 * @brief support for multi-thread, but not recommand.
 *
 * @tparam Key
 * @tparam Val
 * @tparam PageSize
 * @tparam IndexType
 */
template <
    typename Key,
    typename Val,
    std::size_t PageSize = internal::default_page_size,
    typename IndexType   = std::size_t>
class shared_sparse_map final {
public:
    using self_type      = shared_sparse_map;
    using key_type       = Key;
    using mapped_type    = Val;
    using map_type       = sparse_map<key_type, mapped_type, PageSize, IndexType>;
    using iterator       = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;

    shared_sparse_map() : map_() {}
    shared_sparse_map(const shared_sparse_map& other) : map_(other.map_) {}

    ~shared_sparse_map() = default;

    shared_sparse_map(shared_sparse_map&& other) noexcept : map_(std::move(other.map_)), mutex_() {}
    shared_sparse_map& operator=(const shared_sparse_map& other) {
        if (this == &other) {
            return *this;
        }

        map_ = other.map_;

        return *this;
    }
    shared_sparse_map& operator=(shared_sparse_map&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        map_ = std::move(other.map_);

        return *this;
    }

    [[nodiscard]] mapped_type& at(const key_type key) {
        std::shared_lock<const std::shared_mutex> lock(mutex_);
        return map_.at(key);
    }

    [[nodiscard]] const mapped_type& at(const key_type key) const {
        std::shared_lock<const std::shared_mutex> lock(mutex_);
        return map_.at(key);
    }

    [[nodiscard]] inline const mapped_type& operator[](const key_type key) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        return map_[key];
    }

    void clear() noexcept {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        map_.clear();
    }

    [[nodiscard]] inline bool empty() const noexcept {
        std::shared_lock<const std::shared_mutex> lock(mutex_);
        return map_.empty();
    }

    inline IndexType size() const noexcept {
        std::shared_lock<const std::shared_mutex> lock(mutex_);
        return map_.size();
    }

    inline auto begin() noexcept {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return map_.begin();
    }
    inline auto cbegin() const noexcept {
        std::shared_lock<const std::shared_mutex> lock(mutex_);
        return map_.cbegin();
    }
    inline auto rbegin() noexcept {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return map_.rbegin();
    }
    inline auto crbegin() const noexcept {
        std::shared_lock<const std::shared_mutex> lock(mutex_);
        return map_.crbegin();
    }
    inline auto end() noexcept {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return map_.end();
    }
    inline auto cend() const noexcept {
        std::shared_lock<const std::shared_mutex> lock(mutex_);
        return map_.cend();
    }
    inline auto rend() noexcept {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return map_.rend();
    }
    inline auto crend() const noexcept {
        std::shared_lock<const std::shared_mutex> lock(mutex_);
        return map_.crend();
    }

private:
    map_type map_;
    std::shared_mutex mutex_;
};
} // namespace atom::utils
