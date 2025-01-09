#pragma once
#include <algorithm>
#include <ranges>
#include <type_traits>
#include "initializer.h"
#include "type.h"
#include "type_traits.h"

namespace atom::utils {

template <typename>
class delegate;

template <typename Ret, typename... Args>
class delegate<Ret(Args...)> final {
public:
    using self_type     = delegate;
    using return_type   = Ret;
    using function_type = Ret(void const*, Args...);
    using type          = Ret(Args...);

    delegate() noexcept : context_(nullptr), function_(nullptr) {}
    delegate(const delegate& other) noexcept = default;
    delegate(delegate&& other) noexcept      = default;
    virtual ~delegate()                      = default;

    template <auto Candidate>
    explicit delegate(utils::spreader<Candidate> spreader) noexcept : context_(nullptr) {
        bind<Candidate>();
    }

    template <auto Candidate, typename Type>
    explicit delegate(utils::spreader<Candidate> spreader, Type& instance) noexcept
        : context_(nullptr) {
        bind<Candidate>(instance);
    }

    explicit delegate(function_type* function, void const* payload) : context_(nullptr) {
        bind(function, payload);
    }

    /**
     * @brief
     *
     * @tparam Candidate
     */
    template <auto Candidate>
    void bind() noexcept {
        static_assert(
            std::is_invocable_r_v<Ret, decltype(Candidate), Args...>,
            "'Candidate' should be a complete function."
        );

        context_  = nullptr;
        function_ = [](void const*, Args... args) -> Ret {
            return Ret(std::invoke(Candidate, std::forward<Args>(args)...));
        };
    }

    /**
     * @brief
     *
     * @tparam Candidate
     * @tparam Type
     * @param instance
     */
    template <auto Candidate, typename Type>
    void bind(Type& instance) noexcept {
        static_assert(
            std::is_invocable_r_v<Ret, decltype(Candidate), Type*, Args...>,
            "'Candidate' should be a complete function."
        );

        context_  = &instance;
        function_ = [](void const* payload, Args... args) -> Ret {
            Type* type_instance =
                static_cast<Type*>(const_cast<utils::same_constness_t<void, Type>*>(payload));
            return Ret(std::invoke(Candidate, *type_instance, std::forward<Args>(args)...));
        };
    }

    /**
     * @brief Bind function to this delegate
     *
     * @param function Pointer to the function
     * @param payload
     */
    void bind(function_type* function, void const* payload) {
        assert(function);
        function_ = function;
        context_  = payload;
    }

    /**
     * @brief Call function has already bind
     *
     * @param args Arguments
     * @return Ret Function's return type
     */
    Ret operator()(Args... args) const { return function_(context_, std::forward<Args>(args)...); }

    operator bool() const noexcept { return function_; }

    delegate& operator=(const delegate& other) noexcept {
        // type of `function_` and `context_` are pointer
        // do not check self assignment
        function_ = other.function_;
        context_  = other.context_;
        return *this;
    }

    delegate& operator=(delegate&& other) noexcept {
        // copy pointer, no check
        function_ = other.function_;
        context_  = other.context_;

        return *this;
    }

    template <typename Type>
    bool operator==(delegate<Type>& other) const noexcept {
        if constexpr (std::is_same_v<Ret(Args...), Type>) {
            return function_ == other._function && context_ == other._context;
        }
        else {
            return false;
        }
    }

    template <typename Type>
    bool operator!=(delegate<Type>& other) const noexcept {
        return !(*this == other);
    }

    /**
     * @brief Clear bindings
     *
     */
    void reset() noexcept {
        context_  = nullptr;
        function_ = nullptr;
    }

    /**
     * @brief Get the function pointer
     *
     * @return function_type* Pointer of function
     */
    [[nodiscard]] function_type* target() const noexcept { return function_; }

    /**
     * @brief Get the context
     *
     * @return void const* Pointer to the context
     */
    [[nodiscard]] void const* context() const noexcept { return context_; }

private:
    void const* context_;
    function_type* function_;
};

class basic_sink {
    friend class dispatcher;

public:
    using self_type = basic_sink;

    virtual ~basic_sink()                    = default;
    basic_sink() noexcept                    = default;
    basic_sink(const basic_sink&)            = default;
    basic_sink(basic_sink&&)                 = default;
    basic_sink& operator=(const basic_sink&) = default;
    basic_sink& operator=(basic_sink&&)      = default;
    virtual void trigger(void*) const        = 0;
};

template <typename EventType>
class sink final : public basic_sink {
    friend class dispatcher;

    using default_id_t = utils::default_id_t;

public:
    using self_type     = sink;
    using event_type    = EventType;
    using delegate_type = delegate<void(EventType&)>;

    sink() = default;

    template <auto Candidate>
    void connect() {
        default_id_t delegate_id = utils::non_type::id<delegate_type, Candidate>();

        if (auto iter = delegates_.find(delegate_id); iter == delegates_.cend()) {
            delegates_.emplace(delegate_id, delegate_type(utils::spread_arg<Candidate>));
        }
        else {
            iter->second.template bind<Candidate>();
        }
    }

    template <auto Candidate, typename Type>
    void connect(Type& instance) {
        default_id_t delegate_id = utils::non_type::id<delegate_type, Candidate>();

        if (auto iter = delegates_.find(delegate_id); iter == delegates_.cend()) {
            delegates_.emplace(delegate_id, delegate_type(utils::spread_arg<Candidate>, instance));
        }
        else {
            iter->second.template bind<Candidate>(instance);
        }
    }

    template <auto Candidate>
    void disconnect() {
        default_id_t delegate_id = utils::non_type::id<delegate_type, Candidate>();

        if (auto iter = delegates_.find(delegate_id); iter != delegates_.cend()) {
            delegates_.erase(iter);
        }
    }

    template <auto Candidate, typename Type>
    void disconnect() {
        default_id_t delegate_id = utils::non_type::id<delegate_type, Candidate>();

        if (auto iter = delegates_.find(delegate_id); iter != delegates_.cend()) {
            delegates_.erase(iter);
        }
    }

    void trigger(void* event) const override {
        std::ranges::for_each(delegates_, [event](const auto& pair) {
            pair.second(*(static_cast<EventType*>(event)));
        });
    }

    std::unordered_map<default_id_t, delegate_type> delegates_;
};

class dispatcher final {
public:
    using self_type = dispatcher;

    dispatcher() = default;

    virtual ~dispatcher() {
        std::ranges::for_each(sink_map_, [](auto& pair) { delete pair.second; });
    }

    /**
     * @brief Move constructor.
     * std::is_nothrow_move_constructible_v<decltype(sink_map_)> evaluated as false.
     *
     * @param other
     */
    dispatcher(dispatcher&& that) noexcept(false)
        : sink_map_(std::move(that.sink_map_)), events_(std::move(that.events_)) {}

    dispatcher& operator=(dispatcher&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        sink_map_ = std::move(other.sink_map_);
        events_   = std::move(other.events_);

        return *this;
    }

    dispatcher(const dispatcher&) = delete;

    dispatcher& operator=(const dispatcher&) = delete;

    template <typename EventType>
    [[nodiscard]] sink<EventType>& sink() {
        using sink_type = ::atom::utils::sink<EventType>;

        utils::default_id_t type_id = utils::type<dispatcher>::id<EventType>();

        // 如果没找到创建新的就创建一个
        if (auto iter = sink_map_.find(type_id); iter == sink_map_.cend()) {
            sink_map_.emplace(type_id, ::new sink_type());
        }
        return *static_cast<sink_type*>(sink_map_.at(type_id));
    }

    template <typename EventType>
    void trigger(EventType& event) const {
        using sink_type = ::atom::utils::sink<EventType>;

        default_id_t type_id = utils::type<dispatcher>::id<EventType>();

        if (auto iter = sink_map_.find(type_id); iter != sink_map_.cend()) {
            static_cast<sink_type*>(iter->second)->trigger(&event);
        }
    }

    /**
     * @brief Put an event into the queue.
     *
     * @tparam EventType Type of the event. This tparam could be decl by compiler.
     * @param event
     */
    template <typename EventType>
    void enqueue(EventType&& event) {
        using sink_type    = ::atom::utils::sink<EventType>;
        using decayed_type = std::decay_t<EventType>;

        default_id_t type_id = utils::type<dispatcher>::id<std::remove_const_t<decayed_type>>();

        auto ptr = new initializer<std::remove_const_t<decayed_type>, false, lazy>();
        ptr->template init<EventType>(std::forward<EventType>(event));

        events_.emplace_back(type_id, ptr);
    }

    template <typename EventType>
    void update() {
        using sink_type = ::atom::utils::sink<EventType>;

        default_id_t type_id = ::atom::utils::type<dispatcher>::id<EventType>();

        if (auto iter = sink_map_.find(type_id); iter != sink_map_.cend()) {
            auto events =
                events_ |
                std::views::filter([type_id](const auto& pair) { return pair.first == type_id; }) |
                std::views::values;
            std::ranges::for_each(events, [&iter](auto& event) {
                static_cast<sink_type*>(iter->second)->trigger(event);
                delete event;
                event = nullptr;
            });
            std::ranges::remove_if(events_, [](const auto& pair) {
                return pair.second == nullptr;
            });
        }
    }

    void update() {
        auto func = [this](auto& pair) {
            if (auto iter = sink_map_.find(pair.first); iter != sink_map_.cend()) {
                iter->second->trigger(pair.second->raw());
            }

            delete pair.second;
        };
        std::ranges::for_each(events_, func);

        events_.clear();
    }

private:
    // wrapper
    std::unordered_map<utils::default_id_t, basic_sink*> sink_map_;
    std::list<std::pair<default_id_t, basic_initializer*>> events_;
};
} // namespace atom::utils
