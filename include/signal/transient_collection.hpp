#pragma once
#include <cstddef>
#include <ranges>
#include <unordered_map>
#include "core.hpp"
#include "core/pair.hpp"
#include "memory/allocator.hpp"
#include "reflection.hpp"
#include "signal.hpp"
#include "structures.hpp"
#include "structures/dense_map.hpp"

namespace atom::utils {

template <typename Alloc>
class transient_collection {
    template <typename T>
    using allocator_t = typename rebind_allocator<Alloc>::template to<T>::type;
    using alty        = allocator_t<void*>;

public:
    transient_collection() = default;

    transient_collection(const transient_collection& that) = delete;

    transient_collection(transient_collection&& that) noexcept : events_(std::move(events_)) {}

    transient_collection& operator=(const transient_collection& that) = delete;

    transient_collection& operator=(transient_collection&& that) noexcept {
        if (this != &that) {
            events_ = std::move(that.events_);
        }
        return *this;
    }

    ~transient_collection() = default;

    /**
     * @brief
     *
     * @tparam EventType Type of the event. This tparam should be pass by hand.
     * @tparam Event Forwarding reference for event type, which could be deduced.
     * @tparam hash Hash value of EventType could be deduced.
     * @param event
     */
    template <typename EventType, typename Event, size_t hash = hash_of<EventType>()>
    void push(Event&& event) {
        // allocator_t<EventType> allocator{ events_.get_allocator() };
    }

    /**
     * @brief Pop elements.
     * @warning This function should be called at the end of a frame.
     */
    void pop() { events_.clear(); }

    template <typename EventType, size_t hash = hash_of<EventType>()>
    auto filt() noexcept {
        const auto id = identity(hash);

        auto view = std::views::filter(events_, [&id](compressed_pair<default_id_t, void*> pair) {
            return pair.first() == id;
        });
        return view;
    }

private:
    default_id_t identity(size_t hash) {
        static std::unordered_map<size_t, default_id_t> map_;
        if (auto iter = map_.find(hash); iter != map_.end()) [[likely]] {
            return iter->second;
        }
        else {
            const auto id = map_.size();
            map_[hash]    = id;
            return id;
        }
    }

    dense_map<default_id_t, void*, alty, k_default_page_size> events_;
};

} // namespace atom::utils
