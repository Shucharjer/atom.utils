#pragma once
#include <atomic>
#include <type_traits>
#include "core.hpp"

namespace atom::utils {

/**
 * @brief Auxiliary type for passing parameters in template constructors.
 *
 * @tparam auto Argument.
 */
template <auto>
struct spreader {
    explicit spreader() = default;
};

template <auto Candidate>
inline constexpr spreader<Candidate> spread_arg{};

/**
 * @brief Auxiliary type for passing type in template constructors.
 *
 * @tparam typename Type.
 */
template <typename>
struct type_spreader {
    explicit type_spreader() = default;
};

template <typename Type>
inline constexpr type_spreader<Type> spread_type{};

template <typename>
class type final {
public:
    type() = delete;
    /**
     * @brief
     *
     * @tparam Ty
     * @return default_id_t
     */
    template <typename Ty>
    static default_id_t id() {
        return id_<std::remove_cvref_t<Ty>>();
    }

private:
    template <typename Ty>
    static default_id_t id_() {
        static default_id_t type_id = current_id_.fetch_add(1, std::memory_order_acquire);
        return type_id;
    }
    static inline ::std::atomic<default_id_t> current_id_;
};

class non_type {
public:
    template <typename placeholder, auto Param>
    [[nodiscard]] static default_id_t id() {
        static default_id_t type_id = type_id_++;
        return type_id;
    }

private:
    static inline ::std::atomic<default_id_t> type_id_;
};

/**
 * @brief Universal type for deducing.
 *
 * If you define the operator, you may make things confusing!
 * like this: std::vector<int> vec(universal{});
 */
struct universal {
    template <typename Ty>
    operator Ty();
};

} // namespace atom::utils
