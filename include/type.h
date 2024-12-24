#pragma once
#include <iostream>
#include <memory>
#include "./fwd.h"

namespace atom::utils {

template <auto>
struct spreader {
    explicit spreader() = default;
};

template <auto Candidate>
inline constexpr spreader<Candidate> spread_arg{};

template <typename>
struct type_spreader {
    explicit type_spreader() = default;
};

template <typename Type>
inline constexpr type_spreader<Type> spread_type{};

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
        static default_id_t type_id = current_id_++;
        return type_id;
    }

private:
    static inline std::atomic<default_id_t> current_id_;
};

class non_type {
public:
    template <typename placeholder, auto Param>
    [[nodiscard]] static default_id_t id() {
        static default_id_t type_id = type_id_++;
        return type_id;
    }

private:
    static inline std::atomic<default_id_t> type_id_;
};

} // namespace atom::utils
