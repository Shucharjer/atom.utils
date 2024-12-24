#pragma once
#include <exception>
#include <functional>
#include <iostream>
#include <new>
#include <type_traits>
#include "memory_pool.h"

namespace atom::utils {

/**
 * @brief Init Ty lazily
 *
 * @tparam Ty Data type
 * @tparam in_pool Is data create in memory pool?
 */
template <typename Ty, bool in_pool>
class lazy;

/**
 * @brief Init Ty instantly
 *
 * @tparam Ty Data type
 * @tparam in_pool Is data create in memory pool?
 */
template <typename Ty, bool in_pool>
class inst;

/**
 * @brief Base of initializer
 *
 * Empty class.
 * Provide a way to destruct elements in container.
 *
 * Adaptor Pattern
 *
 */
class basic_initializer {
public:
    basic_initializer() noexcept                                    = default;
    basic_initializer(const basic_initializer&) noexcept            = default;
    basic_initializer(basic_initializer&&) noexcept                 = default;
    basic_initializer& operator=(const basic_initializer&) noexcept = default;
    basic_initializer& operator=(basic_initializer&&) noexcept      = default;
    virtual ~basic_initializer() noexcept(false)                    = default;
    [[nodiscard]] virtual auto has_value() const noexcept -> bool { return false; }
    [[nodiscard]] virtual auto raw() -> void* { return nullptr; }
    [[nodiscard]] virtual auto raw() const noexcept -> void* { return nullptr; }
    virtual void release() {}
};

/**
 * @brief A container that store a value and initialize it lazily or instantly.
 *
 * @tparam Ty Type of the value.
 * @tparam in_pool Is its value create in memory pool?
 * @tparam Init Initialization privicy.
 */
template <typename Ty, bool in_pool = true, template <typename, bool> typename Init = lazy>
class initializer;

template <typename Ty, template <typename, bool> typename Init>
class initializer<Ty, false, Init> final : public basic_initializer {
public:
    using self_type  = initializer;
    using init_type  = Init<Ty, false>;
    using value_type = Ty;

    initializer() noexcept(std::is_nothrow_default_constructible_v<init_type>) : init_() {}
    initializer(const initializer& other) noexcept(std::is_nothrow_copy_constructible_v<init_type>)
        : init_(other.init_) {}
    initializer(initializer&& other) noexcept(std::is_nothrow_move_constructible_v<init_type>)
        : init_(std::move(other.init_)) {}
    initializer& operator=(const initializer& other
    ) noexcept(std::is_nothrow_copy_assignable_v<init_type>) {
        init_ = other.init_;
    }
    initializer& operator=(initializer&& other
    ) noexcept(std::is_nothrow_move_assignable_v<init_type>) {
        init_ = std::move(other.init_);
    }
    ~initializer() noexcept(std::is_nothrow_destructible_v<init_type>) override = default;

    /**
     * @brief Initialize the value it stores.
     *
     * This function assume that you have already called has_value(), otherwise there will be
     * some performance loss.
     *
     * @tparam Args Type of arguments to initialize the value
     * @param args Arguments
     */
    template <typename... Args>
    void init(Args&&... args) noexcept(std::is_nothrow_invocable_v<
                                       decltype(&init_type::template init<Args...>),
                                       self_type&,
                                       Args...>) {
        init_.template init<Args...>(std::forward<Args>(args)...);
    }

    [[nodiscard]] auto has_value() const noexcept -> bool override { return init_.has_value(); }
    [[nodiscard]] auto get() -> Ty& { return init_.get(); }
    [[nodiscard]] auto get() const -> Ty& { return init_.get(); }
    [[nodiscard]] auto raw() -> void* override {
        return reinterpret_cast<void*>(std::addressof(init_.get()));
    }
    [[nodiscard]] auto raw() const noexcept -> void* override {
        return reinterpret_cast<void*>(const_cast<Ty*>(std::addressof(init_.get())));
    }
    void release() noexcept(std::is_nothrow_destructible_v<init_type>) override {}

private:
    init_type init_;
};

template <typename Ty, template <typename, bool> typename Init>
class initializer<Ty, true, Init> final : public basic_initializer {
public:
    using self_type  = initializer;
    using init_type  = Init<Ty, true>;
    using value_type = Ty;

    initializer(std::shared_ptr<memory_pool>& pool_ptr
    ) noexcept(std::is_nothrow_default_constructible_v<basic_initializer> && std::is_nothrow_constructible_v<init_type, std::shared_ptr<memory_pool>&>)
        : basic_initializer(), init_(pool_ptr) {}
    initializer(std::shared_ptr<memory_pool>&& pool_ptr
    ) noexcept(std::is_nothrow_default_constructible_v<basic_initializer> && std::is_nothrow_constructible_v<init_type, std::shared_ptr<memory_pool>&&>)
        : basic_initializer(), init_(std::move(pool_ptr)) {}
    initializer(const initializer& other) noexcept(std::is_nothrow_copy_constructible_v<init_type>)
        : init_(other.init_) {}
    initializer(initializer&& other) noexcept(std::is_nothrow_move_constructible_v<init_type>)
        : init_(std::move(other.init_)) {}
    initializer& operator=(const initializer& other
    ) noexcept(std::is_nothrow_copy_assignable_v<init_type>) {
        init_ = other.init_;
    }
    initializer& operator=(initializer&& other
    ) noexcept(std::is_nothrow_move_assignable_v<init_type>) {
        init_ = std::move(other.init_);
    }
    ~initializer() noexcept(std::is_nothrow_invocable_v<decltype(&init_type::release)>) override =
        default;

    /**
     * @brief Initialize the value it stores.
     *
     * This function assume that you have already called has_value(), otherwise there will be
     * some performance loss.
     *
     * @tparam Args Type of arguments to initialize the value
     * @param args Arguments
     */
    template <typename... Args>
    void init(Args&&... args) noexcept(std::is_nothrow_invocable_v<
                                       decltype(std::mem_fn(&init_type::template init<Args...>)),
                                       Args...>) {
        init_.template init<Args...>(std::forward<Args>(args)...);
    }

    [[nodiscard]] auto has_value() const noexcept -> bool override { return init_.has_value(); }
    [[nodiscard]] auto get() -> Ty& { return init_.get(); }
    [[nodiscard]] auto get() const -> const Ty& { return init_.get(); }
    [[nodiscard]] auto raw() -> void* override {
        return reinterpret_cast<void*>(std::addressof(init_.get()));
    }
    [[nodiscard]] auto raw() const noexcept -> void* override {
        return reinterpret_cast<void*>(const_cast<Ty*>(std::addressof(init_.get())));
    }
    void release() noexcept(std::is_nothrow_destructible_v<init_type>) override {}

private:
    init_type init_;
};

//////////////////////////////////////////////////////////////////////////////////////
//                                  Not On Pool
//////////////////////////////////////////////////////////////////////////////////////

template <typename Ty>
class lazy<Ty, false> {
public:
    using self_type  = lazy;
    using value_type = Ty;

    lazy() noexcept : data_(nullptr) {}
    lazy(const lazy& other) noexcept {
        if (other.data_) {
            try {
                data_ = new Ty(*other.data_);
            }
            catch (const std::bad_alloc& exception) {
                std::cerr << "Memory allocation failed: " << exception.what() << std::endl;
            }
        }
        else {
            data_ = nullptr;
        }
    }
    lazy(lazy&& other) noexcept : data_(other.data_) { other.data_ = nullptr; }
    lazy& operator=(const lazy& other) noexcept {
        if (this == &other) {
            return *this;
        }

        if (static_cast<bool>(other.data_)) {
            if (static_cast<bool>(data_)) {
                *data_ = *other.data_;
            }
            else {
                try {
                    data_ = new Ty(*other.data_);
                }
                catch (const std::bad_alloc& exception) {
                    std::cerr << "Memory allocation failed: " << exception.what() << std::endl;
                }
            }
        }
        else {
            if (static_cast<bool>(data_)) {
                delete data_;
                data_ = nullptr;
            }
        }
    }
    lazy& operator=(lazy&& other) noexcept {
        data_       = other.data_;
        other.data_ = nullptr;
    }
    ~lazy() noexcept(std::is_nothrow_invocable_v<decltype(&self_type::release), self_type&>) {
        release();
    }

    /**
     * @brief Initialize the value it stores.
     *
     * This function assume that you have already called has_value(), otherwise there will be
     * some performance loss.
     *
     * @tparam Args Type of arguments to initialize the value
     * @param args Arguments
     */
    template <typename... Args>
    void init(Args&&... args) noexcept(std::is_nothrow_constructible_v<Ty, Args...>) {
        if (!static_cast<bool>(data_)) [[likely]] {
            try {
                data_ = new Ty(std::forward<Args>(args)...);
            }
            catch (const std::bad_alloc& exception) {
                std::cerr << "Memory allocation failed: " << exception.what() << std::endl;
            }
        }
    }

    [[nodiscard]] auto has_value() const noexcept -> bool { return static_cast<bool>(data_); }
    [[nodiscard]] auto get() noexcept -> Ty& {
        if (!static_cast<bool>(data_)) {
            try {
                data_ = new Ty();
            }
            catch (const std::bad_alloc& exception) {
                std::cerr << "Memory allocation failed: " << exception.what() << std::endl;
            }
        }

        return *data_;
    }

    /**
     * @brief Get the object reference.
     *
     * It will cause exception when do not having value.
     */
    [[nodiscard]] auto get() const -> const Ty& { return *data_; }

    void release() noexcept(std::is_nothrow_destructible_v<Ty>) {
        if (static_cast<bool>(data_)) [[likely]] {
            delete data_;
            data_ = nullptr;
        }
    }

private:
    Ty* data_;
};

template <typename Ty>
class inst<Ty, false> {
public:
    using self_type  = inst;
    using value_type = Ty;

    inst() noexcept {
        try {
            data_ = new Ty();
        }
        catch (const std::bad_alloc& exception) {
            std::cerr << "Memory allocation failed: " << exception.what() << std::endl;
        }
    }
    template <typename... Args>
    inst(Args&&... args) noexcept {
        init<Args...>(std::forward<Args>(args)...);
    }
    inst(inst&& other) noexcept : data_(other.data_) { other.data_ = nullptr; }
    inst& operator=(inst&& other
    ) noexcept(std::is_nothrow_invocable_v<decltype(std::mem_fn(&self_type::release))>) {
        release();
        data_       = other.data_;
        other.data_ = nullptr;
    }
    inst(const inst& other) noexcept(std::is_nothrow_copy_constructible_v<Ty>) {
        static_assert(std::is_copy_constructible_v<Ty>);

        if (other.has_value()) {
            init<Ty>(*other.data_);
        }
        else {
            data_ = nullptr;
        }
    }
    inst& operator=(const inst& other) noexcept(std::is_nothrow_copy_assignable_v<Ty>) {
        if (this == &other) {
            return *this;
        }

        if (other.has_value()) {
            if (has_value()) {
                *data_ = *other.data_;
            }
            else {
                data_ = init<Ty>(*other.data_);
            }
        }
        else {
            if (has_value()) {
                delete data_;
                data_ = nullptr;
            }
        }

        return *this;
    }
    ~inst() noexcept(std::is_nothrow_invocable_v<decltype(std::mem_fn(&self_type::release))>) {
        release();
    }

    /**
     * @brief Initialize the value it stores.
     *
     * This function assume that you have already called has_value(), otherwise there will be
     * some performance loss.
     *
     * @tparam Args Type of arguments to initialize the value
     * @param args Arguments
     */
    template <typename... Args>
    void init(Args&&... args) noexcept(std::is_nothrow_constructible_v<Ty, Args...>) {
        if (!static_cast<bool>(data_)) [[likely]] {
            try {
                data_ = new Ty(std::forward<Args>(args)...);
            }
            catch (const std::bad_alloc& exception) {
                std::cerr << "Memory allocation failed: " << exception.what() << std::endl;
            }
        }
    }
    [[nodiscard]] auto has_value() const noexcept -> bool { return static_cast<bool>(data_); }
    [[nodiscard]] auto get() -> Ty& {
        if (!static_cast<bool>(data_)) {
            try {
                data_ = new Ty();
            }
            catch (const std::bad_alloc& exception) {
                std::cerr << "Memory allocation failed: " << exception.what() << std::endl;
                throw;
            }
        }
        return *data_;
    }
    [[nodiscard]] auto get() const -> const Ty& { return *data_; }
    void release() noexcept(std::is_nothrow_destructible_v<Ty>) {
        if (static_cast<bool>(data_)) [[likely]] {
            delete data_;
            data_ = nullptr;
        }
    }

private:
    Ty* data_;
};

//////////////////////////////////////////////////////////////////////////////////////
//                                   On Pool
//////////////////////////////////////////////////////////////////////////////////////

// Must store memory_pool by shared_ptr
// raw pointer or reference may cause error!

template <typename Ty>
class lazy<Ty, true> {
public:
    using self_type  = lazy;
    using value_type = Ty;

    lazy(const std::shared_ptr<memory_pool>& pool_ptr
    ) noexcept(std::is_nothrow_copy_constructible_v<std::shared_ptr<memory_pool>>)
        : pool_(pool_ptr) {}
    lazy(lazy&& other) noexcept : data_(other.data_) { other.data_ = nullptr; }
    lazy& operator=(lazy&& other) noexcept {
        data_       = other.data_;
        other.data_ = nullptr;
    }
    lazy(const lazy& other) : pool_(other.pool_) {
        if (other.data_) {
            construct(*other.data_);
        }
    }
    lazy& operator=(const lazy& other) {
        if (this == &other) {
            return *this;
        }

        if (other.data_) {
            if (data_) {
                assign(*other.data_);
            }
            else {
                construct(*other.data_);
            }
        }
        else {
            data_ = nullptr;
        }
    }
    ~lazy() noexcept(std::is_nothrow_invocable_v<decltype(std::mem_fn(&self_type::release))>) {
        release();
    }

    /**
     * @brief Initialize the value it stores.
     *
     * This function assume that you have already called has_value(), otherwise there will be
     * some performance loss.
     *
     * @tparam Args Type of arguments to initialize the value
     * @param args Arguments
     */
    template <typename... Args>
    void init(Args&&... args) {
        if (!static_cast<bool>(data_)) [[likely]] {
            data_ = pool_->alloc<Ty, Args...>(std::forward<Args>(args)...);
        }
    }
    [[nodiscard]] auto has_value() const -> bool { return static_cast<bool>(data_); }
    [[nodiscard]] auto get() -> Ty& {
        if (data_) {
            return *data_;
        }

        try {
            data_ = pool_->alloc<Ty>();
        }
        catch (...) {
            throw;
        }

        return *data_;
    }
    [[nodiscard]] auto get() const -> const Ty& {
        if (data_) {
            return *data_;
        }

        throw std::exception("try get reference that has not initialized.");
    }

    void release(
    ) noexcept(std::is_nothrow_invocable_v<decltype(std::mem_fn(&memory_pool::dealloc<Ty>)), Ty*>) {
        if (static_cast<bool>(data_)) {
            try {
                pool_->release(data_);
            }
            catch (...) {
                pool_->dealloc(data_);
            }
            data_ = nullptr;
        }
    }

private:
    void assign(Ty& val) {
        if constexpr (std::is_copy_assignable_v<Ty>) {
            *data_ = val;
        }
        else if constexpr (std::is_copy_constructible_v<Ty> && std::is_move_assignable_v<Ty>) {
            Ty temp(val);
            *data_ = std::move(temp);
        }
        else {
            static_assert(false, "'Ty' could not be copy by any way.");
        }
    }

    void construct(Ty& val) {
        if constexpr (std::is_copy_constructible_v<Ty>) {
            data_ = new Ty(val);
        }
        else if (std::is_copy_assignable_v<Ty>) {
            data_  = new Ty;
            *data_ = val;
        }
        else {
            static_assert(false, "'Ty' could not be conustuct as a specific value by any way.");
        }
    }

    Ty* data_{};
    std::shared_ptr<memory_pool> pool_;
};

template <typename Ty>
class inst<Ty, true> {
public:
    using self_type  = inst;
    using value_type = Ty;

    template <typename... Args>
    inst(const std::shared_ptr<memory_pool>& pool_ptr, Args&&... args)
        : pool_(pool_ptr), data_(pool_ptr->alloc<Ty, Args...>(std::forward<Args>(args)...)) {}
    inst(inst&& other) noexcept : data_(other.data_), pool_(std::move(other.pool_)) {
        other.data_ = nullptr;
    }
    inst& operator=(inst&& other
    ) noexcept(std::is_nothrow_invocable_v<decltype(std::mem_fn(&memory_pool::dealloc<Ty>)), Ty*>) {
        if (this == &other) {
            return *this;
        }

        if (static_cast<bool>(data_)) {
            try {
                pool_->release(data_);
            }
            catch (...) {
                pool_->dealloc(data_);
            }
            data_ = other.data_;
        }
        pool_ = std::move(other.pool_);

        return *this;
    }
    inst(const inst& other) : pool_(other.pool_) {}
    inst& operator=(const inst& other) {
        if (this == &other) {
            return *this;
        }

        if (other.data_) [[likely]] {
            if (data_) {
                assign(*other.data_);
            }
            else {
                construct(*other.data_);
            }
        }
        else [[unlikely]] {
            data_ = nullptr;
        }
    }
    ~inst() noexcept(std::is_nothrow_invocable_v<decltype(std::mem_fn(&self_type::release))>) {
        release();
    }

    /**
     * @brief Initialize the value it stores.
     *
     * This function assume that you have already called has_value(), otherwise there will be
     * some performance loss.
     *
     * @tparam Args Type of arguments to initialize the value
     * @param args Arguments
     */
    template <typename... Args>
    void init(Args&&... args) {
        if (!static_cast<bool>(data_)) [[likely]] {
            data_ = pool_->alloc<Ty, Args...>(std::forward<Args>(args)...);
        }
    }
    [[nodiscard]] constexpr auto has_value() const -> bool { return true; }
    [[nodiscard]] auto get() -> Ty& {
        if (data_) {
            return *data_;
        }

        data_ = pool_->alloc<Ty>();

        return *data_;
    }
    [[nodiscard]] auto get() const -> const Ty& { return *data_; }

    void release(
    ) noexcept(std::is_nothrow_invocable_v<decltype(std::mem_fn(&memory_pool::dealloc<Ty>)), Ty*>) {
        if (static_cast<bool>(data_)) {
            try {
                pool_->release(data_);
            }
            catch (...) {
                pool_->dealloc<Ty>(data_);
            }
            data_ = nullptr;
        }
    }

private:
    void assign(Ty& val) {
        if constexpr (std::is_copy_assignable_v<Ty>) {
            *data_ = val;
        }
        else if constexpr (std::is_copy_constructible_v<Ty> && std::is_move_assignable_v<Ty>) {
            Ty temp(val);
            *data_ = std::move(temp);
        }
        else {
            static_assert(false, "'Ty' could not be assign by any way.");
        }
    }

    void construct(Ty& val) {
        if constexpr (std::is_copy_constructible_v<Ty>) {
            data_ = pool_->alloc<Ty, const Ty&>(val);
        }
        else if constexpr (std::is_copy_assignable_v<Ty>) {
            data_  = pool_->alloc<Ty>();
            *data_ = *val;
        }
        else {
            static_assert(false, "'Ty' could not be conustuct as a specific value by any way.");
        }
    }
    Ty* data_;
    std::shared_ptr<memory_pool> pool_;
};

} // namespace atom::utils
