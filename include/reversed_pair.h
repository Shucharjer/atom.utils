#pragma once
#include "compressed_pair.h"

namespace atom::utils {

template <typename First, typename Second>
class reversed_compressed_pair final : private internal::compressed_element<First, true>,
                                       private internal::compressed_element<Second, false> {
public:
    using self_type   = reversed_compressed_pair;
    using first_base  = internal::compressed_element<Second, false>;
    using second_base = internal::compressed_element<First, true>;

    constexpr reversed_compressed_pair(
    ) noexcept(std::is_nothrow_default_constructible_v<second_base> && std::is_nothrow_default_constructible_v<first_base>)
        : second_base(), first_base() {}

    template <typename FirstType, typename SecondType>
    constexpr reversed_compressed_pair(
        FirstType&& first, SecondType&& second
    ) noexcept(std::is_nothrow_constructible_v<first_base, FirstType> && std::is_nothrow_constructible_v<second_base, SecondType>)
        : first_base(std::forward<FirstType>(first)),
          second_base(std::forward<SecondType>(second)) {}

    constexpr reversed_compressed_pair(const compressed_pair&)   = default;
    constexpr compressed_pair& operator=(const compressed_pair&) = default;

    constexpr reversed_compressed_pair(compressed_pair&& that
    ) noexcept(std::is_nothrow_move_constructible_v<first_base> && std::is_nothrow_move_constructible_v<second_base>)
        : first_base(std::move(static_cast<first_base&>(that))),
          second_base(std::move(static_cast<second_base&>(that))) {}

    constexpr reversed_compressed_pair& operator=(compressed_pair&& that
    ) noexcept(std::is_nothrow_move_assignable_v<first_base> && std::is_nothrow_move_assignable_v<second_base>) {
        first()  = std::move(that.first());
        second() = std::move(that.second());
        return *this;
    }

    constexpr ~reversed_compressed_pair(
    ) noexcept(std::is_nothrow_destructible_v<first_base> && std::is_nothrow_destructible_v<second_base>)
        override = default;

    [[nodiscard]] constexpr First& first() noexcept {
        return static_cast<first_base&>(*this).get();
    }

    [[nodiscard]] constexpr const First& first() const noexcept {
        return static_cast<const first_base&>(*this).get();
    }

    [[nodiscard]] constexpr Second& second() noexcept {
        return static_cast<second_base&>(*this).get();
    }

    [[nodiscard]] constexpr const Second& second() const noexcept {
        return static_cast<const second_base&>(*this).get();
    }

    [[nodiscard]] constexpr auto to_pair() -> std::pair<First, Second> {
        return std::make_pair<First&, Second&>(first(), second());
    }

    [[nodiscard]] constexpr auto to_pair() const -> const std::pair<const First&, const Second&> {
        return std::make_pair<const First&, const Second&>(first(), second());
    }
};

template <typename First, typename Second>
struct reversed_pair {
    Second first;
    First second;
};

} // namespace atom::utils
