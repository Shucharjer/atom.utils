#pragma once
#include <atomic>
#include <ranges>
#include <shared_mutex>
#include <unordered_map>
#include "core.hpp"
#include "reflection/hash.hpp"
#include "reflection/reflected.hpp"

namespace atom::utils {

/**
 * @brief Type information registry.
 *
 * Gather reflected information.
 * @tparam BasicConstexprExtend
 * @tparam ConstexprExtend
 */
template <typename Placeholder>
class registry {
    using self_type = registry;
    using pointer   = std::shared_ptr<basic_reflected>;

public:
    registry() = delete;

    /**
     * @brief Get unique identity.
     *
     * @param hash The hash value of a type.
     * @return default_id_t Unique identity.
     */
    static default_id_t identity(const size_t hash) {
        static std::unordered_map<size_t, default_id_t> map;
        if (!map.contains(hash)) {
            map[hash] = next_id();
        }

        return map.at(hash);
    }

    /**
     * @brief Register a type.
     *
     * @tparam Ty The type want to register.
     */
    template <typename Ty>
    static void enroll() {
        using pure_t     = typename std::remove_cvref_t<Ty>;
        const auto hash  = UTILS hash_of<pure_t>();
        const auto ident = identity(hash);

        auto& registered = self_type::registered();
        auto& mutex      = self_type::mutex();
        std::shared_lock<std::shared_mutex> shared_lock{ mutex };
        if (!registered.contains(ident)) [[likely]] {
            shared_lock.unlock();
            std::unique_lock<std::shared_mutex> unique_lock{ mutex };
            registered.emplace(ident, std::make_shared<reflected<pure_t>>());
            unique_lock.unlock();
            shared_lock.lock();
        }
    }

    /**
     * @brief Fint out the reflected information.
     *
     * @param ident Unique identity of a type.
     * @return auto Shared pointer.
     */
    static auto find(const default_id_t ident) -> std::shared_ptr<::atom::utils::basic_reflected> {
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
    static auto find() -> std::shared_ptr<basic_reflected> {
        using pure_t     = std::remove_cvref_t<Ty>;
        const auto hash  = UTILS hash_of<pure_t>();
        const auto ident = identity(hash);
        return find(ident);
    }

    static auto all() {
        auto& registered = self_type::registered();
        return registered | std::views::values;
    }

protected:
    static inline auto registered()
        -> std::unordered_map<default_id_t, std::shared_ptr<basic_reflected>>& {
        static std::unordered_map<default_id_t, pointer> registered;
        return registered;
    }

    static inline auto mutex() -> std::shared_mutex& {
        static std::shared_mutex mutex;
        return mutex;
    }

    static inline auto next_id() -> default_id_t {
        static std::atomic<default_id_t> current_id;
        auto next_id = current_id.load(std::memory_order_acquire);
        current_id.fetch_add(1, std::memory_order_release);
        return next_id;
    }
};

} // namespace atom::utils
