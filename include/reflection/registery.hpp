#pragma once
#include <atomic>
#include <ranges>
#include <shared_mutex>
#include <unordered_map>
#include <utility>
#include "reflection/field_traits.hpp"
#include "reflection/function_traits.hpp"
#include "reflection/hash.hpp"
#include "reflection/reflected.hpp"

namespace atom::utils {

template <typename BasicConstexprExtend, template <typename> typename ConstexprExtend>
class registry {
public:
    using basic_reflected_type = ::atom::utils::basic_reflected<BasicConstexprExtend>;

private:
    using self_type = registry;
    using pointer   = std::shared_ptr<basic_reflected_type>;

public:
    registry() = delete;

    template <typename Ty>
    static void enroll() {
        using pure_t = typename std::remove_cvref_t<Ty>;
        auto hash    = UTILS hash<pure_t>();

        auto& registered = self_type::registered();
        auto& mutex      = self_type::mutex();
        std::shared_lock<std::shared_mutex> shared_lock{ mutex };
        if (!registered.contains(hash)) [[likely]] {
            shared_lock.unlock();
            std::unique_lock<std::shared_mutex> unique_lock{ mutex };
            registered.emplace(
                hash,
                std::make_pair(
                    next_id(),
                    std::make_shared<reflected<pure_t, BasicConstexprExtend, ConstexprExtend>>()
                )
            );
        }
    }

    template <typename Ty>
    static auto find() {
        auto hash = UTILS hash<Ty>();

        auto& registered = self_type::registered();
        auto& mutex      = self_type::mutex();
        std::shared_lock<std::shared_mutex> shared_lock{ mutex };
        if (registered.contains(hash)) [[likely]] {
            return registered.at(hash);
        }
        else [[unlikely]] {
            throw std::runtime_error("Unregistered type!");
        }
    }

    static auto all() {
        auto& registered = self_type::registered();
        return registered | std::views::values;
    }

protected:
    static auto registered() -> std::unordered_map<
        std::size_t,
        std::pair<
            default_id_t,
            std::shared_ptr<::atom::utils::basic_reflected<BasicConstexprExtend>>>>& {
        static std::unordered_map<std::size_t, std::pair<default_id_t, pointer>> registered;
        return registered;
    }

    static auto mutex() -> std::shared_mutex& {
        static std::shared_mutex mutex;
        return mutex;
    }

    static auto next_id() -> default_id_t {
        static std::atomic<default_id_t> current_id;
        auto next_id = current_id.load(std::memory_order_acquire);
        current_id.fetch_add(1, std::memory_order_release);
        return next_id;
    }
};

} // namespace atom::utils
