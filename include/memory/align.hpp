#pragma once
#include <array>
#include <cstddef>
#include <new>

namespace atom::utils {

/**
 * @brief Structure contains aligned value with padding.
 *
 * @note N should be power of 2.
 * @tparam Ty Value's type.
 * @tparam N Alignment. Usually set this equal to or greater than
 * std::hardware_destructive_interference_size, but equal to or little than
 * std::hardware_constructive_interference_size.
 */
template <typename Ty, std::size_t N = std::hardware_destructive_interference_size>
struct alignas(N) aligned {
    Ty value;
    /**
     * @brief Padding
     * For most time, this is not useful.
     */
    std::array<std::byte, N - sizeof(Ty)> _padding;
};

} // namespace atom::utils
