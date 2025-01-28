#pragma once
#include <atomic>
#include <ranges>
#include <shared_mutex>
#include <unordered_map>
#include "core.hpp"
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

    static default_id_t identity(const size_t hash) {
        static std::unordered_map<size_t, default_id_t> map;
        if (!map.contains(hash)) {
            map[hash] = next_id();
        }

        return map.at(hash);
    }

    template <typename Ty>
    static void enroll() {
        using pure_t     = typename std::remove_cvref_t<Ty>;
        const auto hash  = UTILS hash<pure_t>();
        const auto ident = identity(hash);

        auto& registered = self_type::registered();
        auto& mutex      = self_type::mutex();
        std::shared_lock<std::shared_mutex> shared_lock{ mutex };
        if (!registered.contains(ident)) [[likely]] {
            shared_lock.unlock();
            std::unique_lock<std::shared_mutex> unique_lock{ mutex };
            registered.emplace(
                ident, std::make_shared<reflected<pure_t, BasicConstexprExtend, ConstexprExtend>>()
            );
            unique_lock.unlock();
            shared_lock.lock();
        }
    }

    static auto find(default_id_t ident) {
        auto& registered = self_type::registered();
        auto& mutex      = self_type::mutex();
        std::shared_lock<std::shared_mutex> shared_lock{ mutex };
        if (auto iter = registered.find(ident); iter != registered.cend()) [[likely]] {
            return iter->second;
        }
        else [[unlikely]] {
            // Remember some basic classes were not registered automatically, like uint32_t (usually
            // aka unsigned int). You could see these in reflection/macros.hpp
            //
            // Notice: Please make sure you have already registered the type you want to reflect
            // when calling this.
            throw std::runtime_error("Unregistered type!");
        }
    }

    template <typename Ty>
    static auto find() {
        using pure_t     = std::remove_cvref_t<Ty>;
        const auto hash  = UTILS hash<pure_t>();
        const auto ident = identity(hash);
        return find(ident);
    }

    static auto all() {
        auto& registered = self_type::registered();
        return registered | std::views::values;
    }

protected:
    static auto registered() -> std::unordered_map<
        default_id_t,
        std::shared_ptr<::atom::utils::basic_reflected<BasicConstexprExtend>>>& {
        static std::unordered_map<default_id_t, pointer> registered;
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
