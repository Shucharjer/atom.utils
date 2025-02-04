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

public:
    using self_type = dispatcher;

    dispatcher() = default;

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
        using sink_type    = ::atom::utils::sink<EventType>;
        using decayed_type = std::decay_t<EventType>;

        default_id_t type_id =
            utils::type<dispatcher>::template id<std::remove_const_t<decayed_type>>();

        // auto ptr = new initializer<std::remove_const_t<decayed_type>, false, lazy>();
        // ptr->template init<EventType>(std::forward<EventType>(event));

        // events_.emplace_back(type_id, ptr);
    }

    template <typename EventType>
    void update() {
        using sink_type = ::atom::utils::sink<EventType>;

        default_id_t type_id = ::atom::utils::type<dispatcher>::template id<EventType>();

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
        default_id_t,
        basic_sink*,
        std::hash<default_id_t>,
        std::equal_to<default_id_t>,
        allocator_t<std::pair<const default_id_t, basic_sink*>>>
        sink_map_;
    std::list<
        compressed_pair<default_id_t, basic_storage*>,
        allocator_t<compressed_pair<default_id_t, basic_storage*>>>
        events_;
};

} // namespace atom::utils
