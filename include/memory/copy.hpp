#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <utility>
#include <immintrin.h>

namespace atom::utils {
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
// NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
namespace internal {
static inline void copy_in_128(uint8_t* dst, const uint8_t* src, uint8_t length) noexcept {
    if (length < 0x4) {
        switch (length) {
        case 0x3:
            *(dst + 2) = *(dst + 2);
        case 0x2:
            *(dst + 1) = *(dst + 1);
        case 0x1:
            *dst = *src;
        default:
            break;
        }
    }
    else if (length & 0x8) {
        *(uint64_t*)dst                  = *(uint64_t*)src;
        *(uint64_t*)(dst + length - 0x8) = *(uint64_t*)(src + length - 0x8);
    }
    else {
        *(uint32_t*)dst                  = *(uint32_t*)src;
        *(uint32_t*)(dst + length - 0x4) = *(uint32_t*)(src + length - 0x4);
    }
}
} // namespace internal
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

#if defined(__clang__)
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
/**
 * @brief Copy fastly.
 * @details It' a macro when compiling with clang.
 */
    #define fastcpy(dst, src, length) __builtin_memcpy(dst, src, length)
// NOLINTEND(cppcoreguidelines-macro-usage)
#elif defined(__GNUC__)
// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
// NOLINTBEGIN(cppcoreguidelines-pro-type-cstyle-cast)
// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)

/**
 * @brief Copy fastly.
 * @details It's faster than __builtin_memcpy when compiling with gcc.
 */
inline void fastcpy(void* dst, const void* src, uint64_t length) noexcept {
    auto* dst_8_1 = (uint8_t*)dst;
    auto* src_8_1 = (const uint8_t*)src;
    if (length < 0x10) { // 128, 16
        internal::copy_in_128(dst_8_1, src_8_1, length);
    }
    else if (length <= 0x20) { // 256, 32
        _mm_storeu_si128((__m128i*)dst, _mm_loadu_si128((const __m128i*)src));
        _mm_storeu_si128(
            (__m128i*)(dst_8_1 + length - 0x10),
            _mm_loadu_si128((const __m128i*)(src_8_1 + length - 0x10)));
    }
    else {
        if (length <= 0x40) { // 512, 64
    #if !defined(__SCE__) || defined(__AVX__)
            _mm256_storeu_si256((__m256i*)dst, _mm256_loadu_si256((const __m256i*)src));
            _mm256_storeu_si256(
                (__m256i*)(dst_8_1 + 0x20), _mm256_loadu_si256((const __m256i*)(src_8_1 + 0x20)));
    #else
    #endif
        }
        else {
            uint64_t n = length >> 6;
    #if !defined(__SCE__) || defined(__AVX__)
            uint64_t offset_1   = 0;
            uint64_t offset_2   = 1;
            auto* dst_256       = (__m256i*)dst;
            const auto* src_256 = (const __m256i*)src;
            if (n <= 4) {
                switch (n) {
                case 4:
                    _mm256_storeu_si256(dst_256 + 7, _mm256_loadu_si256(src_256 + 7));
                    _mm256_storeu_si256(dst_256 + 6, _mm256_loadu_si256(src_256 + 6));
                case 3:
                    _mm256_storeu_si256(dst_256 + 5, _mm256_loadu_si256(src_256 + 5));
                    _mm256_storeu_si256(dst_256 + 4, _mm256_loadu_si256(src_256 + 4));
                case 2:
                    _mm256_storeu_si256(dst_256 + 3, _mm256_loadu_si256(src_256 + 3));
                    _mm256_storeu_si256(dst_256 + 2, _mm256_loadu_si256(src_256 + 2));
                default:
                    _mm256_storeu_si256(dst_256 + 1, _mm256_loadu_si256(src_256 + 1));
                    _mm256_storeu_si256(dst_256, _mm256_loadu_si256(src_256));
                    break;
                }
            }
            else {
                for (auto i = 0; i < n - 4; ++i, offset_1 += 2, offset_2 += 2) {
                    __builtin_prefetch(src_256 + offset_1, 0);
                    __builtin_prefetch(dst_256 + offset_1, 1);
                    _mm256_storeu_si256(dst_256 + offset_1, _mm256_loadu_si256(src_256 + offset_1));
                    _mm256_storeu_si256(dst_256 + offset_2, _mm256_loadu_si256(src_256 + offset_2));
                }
                _mm256_storeu_si256(
                    dst_256 + offset_1 + 7, _mm256_loadu_si256(src_256 + offset_1 + 7));
                _mm256_storeu_si256(
                    dst_256 + offset_1 + 6, _mm256_loadu_si256(src_256 + offset_1 + 6));
                _mm256_storeu_si256(
                    dst_256 + offset_1 + 5, _mm256_loadu_si256(src_256 + offset_1 + 5));
                _mm256_storeu_si256(
                    dst_256 + offset_1 + 4, _mm256_loadu_si256(src_256 + offset_1 + 4));
                _mm256_storeu_si256(
                    dst_256 + offset_1 + 3, _mm256_loadu_si256(src_256 + offset_1 + 3));
                _mm256_storeu_si256(
                    dst_256 + offset_1 + 2, _mm256_loadu_si256(src_256 + offset_1 + 2));
                _mm256_storeu_si256(
                    dst_256 + offset_1 + 1, _mm256_loadu_si256(src_256 + offset_1 + 1));
                _mm256_storeu_si256(dst_256 + offset_1, _mm256_loadu_si256(src_256 + offset_1));
            }
            _mm256_storeu_si256(
                (__m256i*)(dst_8_1 + length - 0x40),
                _mm256_loadu_si256((const __m256i*)(src_8_1 + length - 0x40)));
            _mm256_storeu_si256(
                (__m256i*)(dst_8_1 + length - 0x20),
                _mm256_loadu_si256((const __m256i*)(src_8_1 + length - 0x20)));
    #else
    #endif
        }
    }
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
// NOLINTEND(cppcoreguidelines-pro-type-cstyle-cast)
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
#else
#endif

/**
 * @brief Copy assign.
 * Memcpy on runtime, used to assign to trivially assignable type.
 *
 * @param[out] dst Destination.
 * @param[in] src Source.
 */
template <typename Ty>
requires std::is_trivially_copy_assignable_v<Ty>
constexpr inline void copy(Ty* dst, const Ty* src) noexcept {
    if (std::is_constant_evaluated()) [[unlikely]] {
        dst = src;
    }
    else [[likely]] {
        fastcpy(std::addressof(dst), std::addressof(src), sizeof(Ty));
    }
}

/**
 * @brief Copy assign.
 * Memcpy on runtime, used to assign to trivially assignable type.
 *
 * @param[out] dst Destination.
 * @param[in] src Source.
 * @param length Length bytes.
 */
template <typename Ty>
requires std::is_trivially_copy_assignable_v<Ty>
constexpr inline void copy(Ty* dst, const Ty* src, uint64_t length) noexcept {
    if (std::is_constant_evaluated()) [[unlikely]] {
        for (uint64_t i = 0; i < length; ++i) {
            dst = src;
        }
    }
    else [[likely]] {
        fastcpy(std::addressof(dst), std::addressof(src), length);
    }
}

} // namespace atom::utils
