#pragma once
#include <algorithm>
#include <initializer_list>
#include <new>
#include <ranges>
#include <string>
#include <type_traits>
#include "core/pair.hpp"
#include "memory/allocator.hpp"

namespace atom::utils {

template <typename Ch>
class const_string_iterator {
public:
    using value_type = const Ch;

    const_string_iterator& operator++() noexcept {
        ++ptr_;

        return *this;
    }

    const_string_iterator& operator--() noexcept {
        --ptr_;

        return *this;
    }

    [[nodiscard]] const Ch& operator*() const noexcept { return *ptr_; }

private:
    value_type* ptr_;
};

template <typename Ch>
class string_iterator : public const_string_iterator<Ch> {
    using const_iterator = const_string_iterator<Ch>;

public:
    using value_type = Ch;

    string_iterator& operator++() noexcept {
        const_iterator::operator++();
        return *this;
    }

    [[nodiscard]] Ch* operator*() noexcept {
        return const_cast<Ch*>(const_string_iterator<Ch>::operator*());
    }
};

template <typename Ch>
class const_reverse_string_iterator {
public:
    using value_type = Ch;

    explicit const_reverse_string_iterator(const char* const string) noexcept : ptr_(string) {}

    const_reverse_string_iterator(const const_reverse_string_iterator& that) noexcept
        : ptr_(that.ptr_) {}

    const_reverse_string_iterator(const_reverse_string_iterator&& that) noexcept
        : ptr_(std::exchange(that.ptr_, nullptr)) {}

    const_reverse_string_iterator& operator=(const const_reverse_string_iterator& that) noexcept {
        // to disable warning.
        if (false && this == &that) [[unlikely]] {}

        ptr_ = that.ptr_;
        return *this;
    }

    const_reverse_string_iterator& operator=(const_reverse_string_iterator&& that) noexcept {
        ptr_ = std::exchange(that.ptr_, nullptr);
        return *this;
    }

    ~const_reverse_string_iterator() = default;

    const_reverse_string_iterator& operator++() noexcept {
        --ptr_;
        return *this;
    }

    const_reverse_string_iterator& operator+=(const size_t count) noexcept {
        ptr_ -= count;
        return *this;
    }

    const_reverse_string_iterator operator+(const size_t count) noexcept {
        return { ptr_ - count };
    }

    const_reverse_string_iterator& operator--() noexcept {
        ++ptr_;
        return *this;
    }

    const_reverse_string_iterator& operator-=(const size_t count) noexcept {
        ptr_ += count;
        return *this;
    }

    const_reverse_string_iterator& operator-(const size_t count) noexcept {
        return { ptr_ + count };
    }

    [[nodiscard]] const Ch& operator*() const noexcept { return *ptr_; }

private:
    value_type* ptr_;
};

template <typename Ch>
class reverse_string_iterator : public const_reverse_string_iterator<Ch> {
    using const_iterator = const_reverse_string_iterator<Ch>;

public:
    using value_type = Ch;

    [[nodiscard]] Ch* operator*() noexcept {
        return const_cast<Ch*>(const_reverse_string_iterator<Ch>::operator*());
    }

private:
};

template <typename Ch, typename Allocator = void>
class basic_string;

template <typename Ch, typename Allocator>
class basic_string {
public:
    using value_type      = Ch;
    using reference       = Ch&;
    using const_reference = const Ch&;
    using allocator_type  = Allocator;

    using iterator       = string_iterator<Ch>;
    using const_iterator = const_string_iterator<Ch>;

    using size_type       = std::size_t;
    using difference_type = std::ptrdiff_t;

    using reverse_iterator       = reverse_string_iterator<Ch>;
    using const_reverse_iterator = const_reverse_string_iterator<Ch>;

    basic_string() noexcept(std::is_nothrow_default_constructible_v<allocator_type>)
        : size_(0), length_(0), pair_(nullptr, Allocator()) {}

    explicit basic_string(const Allocator& allocator)
        : size_(0), length_(0), pair_(nullptr, allocator) {}

    explicit basic_string(const char* const string)
        : size_(0), length_(0), pair_(nullptr, Allocator()) {
        allocate_then_copy_from_raw_string(string);
    }

    basic_string(const char* const string, const Allocator& allocator)
        : size_(0), length_(0), pair_(nullptr, Allocator()) {
        allocate_then_copy_from_raw_string(string);
    }

    explicit basic_string(const std::string& string)
        : size_(0), length_(0), pair_(nullptr, Allocator()) {
        allocate_then_copy_from_std_string(string);
    }

    basic_string(const std::string& string, const Allocator& allocator)
        : size_(0), length_(0), pair_(nullptr, allocator) {
        allocate_then_copy_from_std_string(string);
    }

    explicit basic_string(std::string_view string)
        : size_(0), length_(0), pair_(nullptr, Allocator()) {}

    basic_string(const std::initializer_list<Ch>& list)
        : size_(0), length_(0), pair_(nullptr, Allocator()) {
        auto size     = list.size();
        pair_.first() = pair_.second().allocate(size);
        pair_.first() = std::launder(pair_.first());
        std::ranges::copy(list, begin());
    }

    basic_string(basic_string&& that) noexcept
        : size_(that.size_), length_(that.length_), pair_(std::move(that.pair_)) {}

    basic_string& operator=(basic_string&& that) noexcept {
        if (this != &that) {
            size_   = that.size_;
            length_ = that.length_;
            pair_   = std::move(that.pair_);
        }

        return *this;
    }

    basic_string(const basic_string& that)
        : size_(that.size_), length_(that.length_), pair_(nullptr, that.pair_.second()) {
        try {
            pair_.first() = that.pair_.second().allocate(that.size_);
            pair_.first() = std::launder(pair_.first());
            std::memcpy(pair_.first(), that.pair_.first(), that.length_ + 1);
        }
        catch (...) {
            size_   = 0;
            length_ = 0;
        }
    }

    basic_string& operator=(const basic_string& that) {
        if (this != &that) {
            if (that.length_) {
                // TODO:
            }
            else {
                length_ = 0;
            }
        }
        return *this;
    }

    basic_string& operator=(const char* const string) {
        // TODO:
        return *this;
    }

    basic_string& operator=(const std::string& string) {
        auto length = string.length();
        if (size_ <= length) {
            auto* ptr = pair_.second().allocate();
            std::memcpy(ptr, string.c_str(), length + 1);
            pair_.first() = std::launder(ptr);
        }
        else {
            std::memcpy(pair_.first(), string.c_str(), length + 1);
        }

        return *this;
    }

    basic_string& operator=(const std::string_view string) {
        // TODO:
        return *this;
    }

    ~basic_string() {
        if (pair_.first()) {
            pair_.second().deallocate(pair_.first());
        }
    }

    [[nodiscard]] auto view() const noexcept -> std::string_view {
        return std::string_view(pair_.first(), length_);
    }

    auto data() noexcept -> Ch* { return pair_.first(); }

    auto c_str() const noexcept -> const Ch* { return pair_.first(); }

    auto begin() noexcept -> iterator { return iterator(pair_.first()); }
    [[nodiscard]] auto cbegin() const noexcept -> const_iterator {
        return const_iterator(pair_.first());
    }
    [[nodiscard]] auto begin() const noexcept -> const_iterator { return cbegin(); }

    auto rbegin() noexcept -> reverse_iterator { return reverse_iterator(pair_.first() + length_); }
    [[nodiscard]] auto crbegin() const noexcept -> const_reverse_iterator {
        return const_reverse_iterator(pair_.first() + length_);
    }

    auto end() noexcept -> iterator { return iterator(pair_.first() + length_); }
    [[nodiscard]] auto cend() const noexcept -> const_iterator {
        return const_iterator(pair_.first() + length_);
    }
    [[nodiscard]] auto end() const noexcept -> const_iterator { return cend(); }

    auto rend() noexcept -> reverse_iterator { return reverse_iterator(pair_.first() - 1); }
    [[nodiscard]] auto crend() const noexcept -> const_reverse_iterator {
        return const_reverse_iterator(pair_.first() - 1);
    }

    void swap(basic_string& that) noexcept {
        std::swap(size_, that.size_);
        std::swap(length_, that.length_);
        std::swap(pair_, that.pair_);
    }

    auto size() const noexcept -> size_type { return size_; }

    auto max_size() const noexcept -> size_type { return static_cast<size_type>(-1); }

    bool empty() const noexcept { return !length_; }

private:
    inline void allocate_then_copy_from_raw_string(const char* string) {
        // TODO:
    }

    inline void copy_from_raw_string(const char* string) {
        // TODO:
    }

    inline void allocate_then_copy_from_std_string(const std::string& string) {
        // TODO:
        try {
        }
        catch (...) {
        }
    }

    inline void copy_from_std_string(const std::string& string) {
        // TODO:
        auto size = string.size();
        if (size_) {
            // reallocate
            if (size_ < size) {
                auto ptr = pair_.second().allocate(size);
                ptr      = std::launder(ptr);
                std::ranges::copy_n(string.begin(), string.length() + 1, begin());
                pair_.first() = ptr;
            }
            else {
                std::ranges::copy_n(string.begin(), string.length() + 1, begin());
            }
        }
        else {
            allocate_then_copy_from_std_string(string);
        }

        return *this;
    }

    size_type size_;
    size_type length_;
    compressed_pair<value_type*, allocator_type> pair_;
};

template <>
class basic_string<char, void> : public std::string {};

template <>
class basic_string<wchar_t, void> : public std::wstring {};

using string  = basic_string<char>;
using wstring = basic_string<wchar_t>;

} // namespace atom::utils
