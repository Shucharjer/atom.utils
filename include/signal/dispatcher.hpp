#pragma once
#include <algorithm>
#include <ranges>
#include "concepts/allocator.hpp"
#include "core.hpp"
#include "core/pair.hpp"
#include "memory.hpp"
#include "memory/allocator.hpp"
#include "memory/storage.hpp"
#include "signal.hpp"
#include "signal/delegate.hpp"
#include "signal/sink.hpp"

namespace atom::utils {

template <concepts::rebindable_allocator Allocator>
class dispatcher final {
    template <typename Target>
    using allocator_t = typename rebind_allocator<Allocator>::template to<Target>::type;

    using sink_map_alloc_t = allocator_t<std::pair<const default_id_t, basic_sink*>>;
    using events_alloc_t   = allocator_t<utils::compressed_pair<default_id_t, void*>>;

public:
    using self_type = dispatcher;

    template <concepts::rebindable_allocator Al = Allocator>
    requires std::is_constructible_v<sink_map_alloc_t, Al> &&
                 std::is_constructible_v<events_alloc_t, Al>
    dispatcher(Al&& allocator = Allocator{})
        : sink_map_(sink_map_alloc_t{ allocator }),
          events_(events_alloc_t{ std::forward<Al>(allocator) }) {}

    ~dispatcher() {
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

        utils::default_id_t type_id = utils::type<dispatcher>::template id<EventType>();

        if (auto iter = sink_map_.find(type_id); iter == sink_map_.cend()) {
            sink_map_.emplace(type_id, ::new sink_type());
        }
        return *static_cast<sink_type*>(sink_map_.at(type_id));
    }

    template <typename EventType>
    void trigger(EventType& event) const {
        using sink_type = ::atom::utils::sink<EventType>;

        default_id_t type_id = utils::type<dispatcher>::template id<EventType>();

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
        using sink_type = ::atom::utils::sink<EventType>;
        using pure      = std::remove_cvref_t<EventType>;

        default_id_t type_id = utils::type<dispatcher>::template id<pure>();
        events_.emplace_back(type_id, new pure(std::forward<EventType>(event)));
    }

    template <typename EventType>
    void update() {
        using sink_type = ::atom::utils::sink<EventType>;

        default_id_t type_id = ::atom::utils::type<dispatcher>::template id<EventType>();

        // find the relative sink.
        if (auto iter = sink_map_.find(type_id); iter != sink_map_.cend()) {
            auto* sink = static_cast<sink_type*>(iter->second);
            // update events for this sink.
            // auto update_relative = [&sink,
            //                         type_id](compressed_pair<const default_id_t, void*>& pair) {
            //     if (pair.first() == type_id) {
            //         sink->trigger(pair.second());
            //         pair.second() = nullptr;
            //     }
            // };
            // std::ranges::for_each(events_, update_relative);
            for (auto& [id, ptr] : events_) {
                if (id == type_id) {
                    sink->trigger(ptr);
                    ptr = nullptr;
                }
            }
            std::ignore = std::ranges::remove_if(
                events_, [](const auto& pair) { return pair.second() == nullptr; });
        }
    }

    void update() {
        auto func = [this](auto& pair) {
            if (auto iter = sink_map_.find(pair.first()); iter != sink_map_.cend()) {
                iter->second->trigger(pair.second()->raw());
            }

            delete pair.second();
        };
        std::ranges::for_each(events_, func);

        events_.clear();
    }

private:
    // wrapper
    std::unordered_map<
        default_id_t, basic_sink*, std::hash<default_id_t>, std::equal_to<default_id_t>,
        allocator_t<std::pair<const default_id_t, basic_sink*>>>
        sink_map_;
    std::list<
        compressed_pair<default_id_t, void*>, allocator_t<compressed_pair<default_id_t, void*>>>
        events_;
};

} // namespace atom::utils
