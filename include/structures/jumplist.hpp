#pragma once

namespace atom::utils {

template <typename Ty>
struct _Jumplist_node {
    uint8_t level;
    _Jumplist_node** next;
    Ty value;

    template <typename... Args>
    constexpr _Jumplist_node(uint8_t level, Args&&... args) noexcept : level(level), value(std::forward<Args>(args)...) {}

    ~_Jumplist_node() noexcept(std::is_nothrow_destructible_v<Ty>) {}
};

template <typename Ty, bool Const>
class _Jumplist_iterator {
    _Jumplist_node<Ty>* ptr_;
public:
    _Jumplist_iterator(_Jumplist_node<Ty>* ptr) noexcept : ptr_(ptr) {}
};

template <typename Ty, size_t Size, typename Alloc = _Inplace_allocator<_Jumplist_node<Ty>, Size>>
class jumplist {
    Alloc allocator_{};
    size_t size_{};
    _Jumplist_node<Ty>* head_{};

    using alty_traits = std::allocator_traits<inplace_allocator<Ty, Size>>;

    template <typename... Args>
    void _Construct_at(_Jumplist_node<Ty>* const ptr, Args&&... args) noexcept (std::is_nothrow_constructible_v<Ty, Args...>) {}
    void _Destroy_at(_Jumplist_node<Ty>* const ptr) noexcept (std::is_nothrow_destructible_v<Ty>) {}
public:
    using value_type = Ty;
    using iterator = _Jumplist_iterator<Ty, false>;
    using const_iterator = _Jumplist_iterator<Ty, true>;

    jumplist() noexcept = default;
    jumplist(const jumplist& that) noexcept(std::is_nothrow_copy_constructible_v<Ty>) {}
    jumplist(jumplist&& that) noexcept(std::is_nothrow_move_constructible_v<Ty>) {}
    jumplist& operator=(const jumplist& that) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {}
    jumplist& operator=(jumplist&& that) noexcept(std::is_nothrow_move_assignable_v<Ty>) {}
    ~jumplist() noexcept(std::is_nothrow_destructible_v<Ty>) {}

    Ty& operator[]() noexcept {}
    const Ty& operator[]() noexcept {}

    iterator begin() noexcept { return head_; }
    const_iterator begin() const noexcept { return head_; }
    iterator end() noexcept { return nullptr; }
    const_iterator end() noexcept { return nullptr; }
};

}
