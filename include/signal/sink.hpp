#pragma once
#include <algorithm>
#include <functional>
#include <type_traits>
#include "core.hpp"
#include "signal.hpp"

namespace atom::utils {

// class basic_sink {
//     template <typename>
//     friend class dispatcher;

// public:
//     using self_type = basic_sink;

//     virtual ~basic_sink()                    = default;
//     basic_sink() noexcept                    = default;
//     basic_sink(const basic_sink&)            = default;
//     basic_sink(basic_sink&&)                 = default;
//     basic_sink& operator=(const basic_sink&) = default;
//     basic_sink& operator=(basic_sink&&)      = default;
//     virtual void trigger(void*) const        = 0;
// };

// template <typename EventType, typename Allocator>
// class sink final : public basic_sink {
//     template <typename>
//     friend class dispatcher;

//     using default_id_t = utils::default_id_t;

// public:
//     using self_type     = sink;
//     using event_type    = EventType;
//     using delegate_type = delegate<void(EventType&)>;

//     template <typename Al = Allocator>
//     requires std::is_constructible_v<
//         std::unordered_map<
//             default_id_t, delegate_type, std::hash<default_id_t>, std::equal_to<default_id_t>,
//             Allocator>,
//         Al>
//     sink(Al&& allocator = Allocator{}) : delegates_(std::forward<Al>(allocator)) {}

//     sink(const sink& that) : delegates_(that.delegates_) {}

//     sink(sink&& that) noexcept : delegates_(std::move(that.delegates_)) {}

//     sink& operator=(const sink& that) { delegates_ = that.delegates_; }

//     sink& operator=(sink&& that) noexcept { delegates_ = std::move(that.delegates_); }

//     ~sink() = default;

//     template <auto Candidate>
//     void connect() {
//         default_id_t delegate_id = utils::non_type::id<delegate_type, Candidate>();

//         if (auto iter = delegates_.find(delegate_id); iter == delegates_.cend()) {
//             delegates_.emplace(delegate_id, delegate_type(utils::spread_arg<Candidate>));
//         }
//         else {
//             iter->second.template bind<Candidate>();
//         }
//     }

//     template <auto Candidate, typename Type>
//     void connect(Type& instance) {
//         default_id_t delegate_id = utils::non_type::id<delegate_type, Candidate>();

//         if (auto iter = delegates_.find(delegate_id); iter == delegates_.cend()) {
//             delegates_.emplace(delegate_id, delegate_type(utils::spread_arg<Candidate>,
//             instance));
//         }
//         else {
//             iter->second.template bind<Candidate>(instance);
//         }
//     }

//     template <auto Candidate>
//     void disconnect() {
//         default_id_t delegate_id = utils::non_type::id<delegate_type, Candidate>();

//         if (auto iter = delegates_.find(delegate_id); iter != delegates_.cend()) {
//             delegates_.erase(iter);
//         }
//     }

//     template <auto Candidate, typename Type>
//     void disconnect() {
//         default_id_t delegate_id = utils::non_type::id<delegate_type, Candidate>();

//         if (auto iter = delegates_.find(delegate_id); iter != delegates_.cend()) {
//             delegates_.erase(iter);
//         }
//     }

//     void trigger(void* event) const override {
//         std::ranges::for_each(delegates_, [event](const auto& pair) {
//             pair.second(*(static_cast<EventType*>(event)));
//         });
//     }

// private:
//     std::unordered_map<
//         default_id_t, delegate_type, std::hash<default_id_t>, std::equal_to<default_id_t>,
//         Allocator>
//         delegates_;
// };

} // namespace atom::utils
