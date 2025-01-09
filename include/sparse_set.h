#pragma once
#include <array>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>
#include "sparse.h"
#include "type_traits.h"

namespace atom::utils {

// 因为在这个类型中Ty是整型，所以传递参数的时候没必要使用引用或移动，但是涉及到写回还是可以用一下左值引用的
template <std::unsigned_integral Ty, std::size_t PageSize, std::unsigned_integral IndexType>
// Ty must be an integral type
requires is_positive_integral_v<PageSize>
class sparse_set {
private:
    [[nodiscard]] constexpr static auto page_(const Ty key) noexcept -> IndexType {
        return key / PageSize;
    }
    [[nodiscard]] constexpr static auto offset_(const Ty key) noexcept -> IndexType {
        return key % PageSize;
    }
    std::vector<Ty> density_;
    std::vector<std::unique_ptr<std::array<IndexType, PageSize>>> sparses_;
    IndexType page_count_;
    IndexType count_;

    template <typename Valty>
    inline auto contains_(const Valty val, const IndexType page, const IndexType offset) const
        -> bool {
        if (!count_ || page >= page_count_) {
            return false;
        }

        switch (sparses_[page]->at(offset)) {
        case 0:
            return density_[0] == val;
        default:
            return true;
        }
        // return !sparses_[page]->at(offset) ? density_[0] == val : true;
    }

    inline void check_page_count_(const IndexType page) {
        auto page_count = page_count_;
        try {
            while (page_count <= page) {
                sparses_.emplace_back(std::make_unique<std::array<IndexType, PageSize>>());
                ++page_count;
            }
        }
        catch (...) {
            while (page_count_ != page_count) {
                sparses_.pop_back();
                --page_count;
            }
            throw;
        }
        page_count_ = page_count;
    }

public:
    using self_type      = sparse_set;
    using value_type     = Ty;
    using iterator       = typename std::vector<value_type>::iterator;
    using const_iterator = typename std::vector<value_type>::const_iterator;

    sparse_set() : page_count_(1), count_(0) {
        sparses_.emplace_back(std::make_unique<std::array<IndexType, PageSize>>());
    }

    ~sparse_set() noexcept = default;

    sparse_set(const sparse_set& obj) {}
    auto operator=(const sparse_set& obj) noexcept -> sparse_set& {
        if (this != &obj) {
            sparse_set set(obj);
            density_    = std::move(set.density_);
            sparses_    = std::move(set.sparses_);
            page_count_ = set.page_count_;
            count_      = set.count_;
        }
        return *this;
    }

    sparse_set(sparse_set&& obj) noexcept
        : page_count_(obj.page_count_), count_(obj.count_), density_(std::move(obj.density_)),
          sparses_(std::move(obj.sparses_)) {}

    auto operator=(sparse_set&& obj) noexcept -> sparse_set& {
        density_.clear();
        sparses_.clear();

        page_count_ = obj.page_count_;
        count_      = obj.count_;
        density_    = std::move(density_);
        sparses_    = std::move(sparses_);

        return *this;
    }

    /**
     * @brief emplace a new element
     *
     * @param val the value of the new element
     */
    void emplace(const Ty val) {
        IndexType page   = page_(val);
        IndexType offset = offset_(val);

        if (contains_(val, page, offset)) {
            return;
        }

        try {
            density_.push_back(std::move(val));
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

    /**
     * @brief return weather the set contains the val
     *
     * @param val the value
     * @return true If contains the value
     * @return false If not
     */
    [[nodiscard]] auto contains(const Ty val) const noexcept -> bool {
        IndexType page   = page_(val);
        IndexType offset = offset_(val);

        return contains_(val, page, offset);
    }

    /**
     * @brief
     *
     * @param val
     */
    void erase(const Ty val) noexcept {
        IndexType page   = page_(val);
        IndexType offset = offset_(val);

        if (!contains_(val, page, offset)) {
            return;
        }

        Ty& final_val           = density_.back();
        IndexType& index_remove = sparses_[page]->at(offset);
        if (val != final_val) {
            IndexType& index_final = sparses_[page_(final_val)]->at(offset_(final_val));
            std::swap(density_[index_remove], final_val);
            std::swap(index_remove, index_final);
        }
        // in fact, if the val is the final val, the fellow line is unuseful
        index_remove = 0;
        density_.pop_back();

        --count_;
    }

    /**
     * @brief
     *
     */
    inline void clear() noexcept {
        density_.clear();
        sparses_.clear();
        page_count_ = 0;
        count_      = 0;
    }

    /**
     * @brief
     *
     * @return IndexType
     */
    [[nodiscard]] inline auto count() const noexcept -> IndexType { return count_; }

    /**
     * @brief
     *
     * @return true
     * @return false
     */
    [[nodiscard]] inline auto empty() const noexcept -> bool { return !count_; }

    /**
     * @brief
     *
     * @param value
     * @return auto
     */
    [[nodiscard]] auto find(const value_type value) const noexcept {
        IndexType page   = page_(value);
        IndexType offset = offset_(value);

        return page < page_count_ &&
                       (sparses_[page]->at(offset) || density_[sparses_[page]->at(offset)] == value)
                   ? density_.begin() +
                         (&(density_[sparses_[page]->at(offset)]) - &(*density_.begin()))
                   : density_.cend();
    }

    /**
     * @brief
     *
     * @param val
     * @return std::optional<IndexType>
     */
    [[nodiscard]] auto index_of(const value_type val) const noexcept -> std::optional<IndexType> {
        IndexType page   = page_(val);
        IndexType offset = offset_(val);

        if (!contains_(val, page, offset)) {
            return std::optional<IndexType>();
        }

        return std::optional<IndexType>(sparses_[page]->at(offset));
    }

    /**
     * @brief
     *
     * @param where
     * @return iterator
     */
    auto erase(const_iterator where) noexcept -> iterator { return density_.erase(where); }

    /**
     * @brief
     *
     * @param val
     * @return value_type&
     */
    [[nodiscard]] auto operator[](const value_type val) -> value_type& {
        auto page   = page_(val);
        auto offset = offset_(val);

        if (!contains_(val, page, offset)) {
            try {
                check_page_count_(page);
                sparses_[page]->at(offset) = count_;
                density_.push_back(std::move(val));
                ++count_;
            }
            catch (...) {
                throw;
            }
        }
        return density_[sparses_[page]->at(offset)];
    }

    inline auto begin() noexcept { return density_.begin(); }
    inline auto cbegin() const noexcept { return density_.cbegin(); }
    inline auto rbegin() noexcept { return density_.rbegin(); }
    inline auto crbegin() const noexcept { return density_.crbegin(); }
    inline auto end() noexcept { return density_.end(); }
    inline auto cend() const noexcept { return density_.cend(); }
    inline auto rend() noexcept { return density_.rend(); }
    inline auto crend() const noexcept { return density_.crend(); }
};
} // namespace atom::utils
