#pragma once
#include <type_traits>

namespace atom::utils {

template <typename Rng, typename Closure>
struct pipeline_result {
    template <typename NextClosure>
    constexpr auto operator|(NextClosure closure) && {
        return pipeline_result<pipeline_result<Rng, Closure>, NextClosure>{ *this, closure };
    }
};

template <typename Rng, typename Closure>
constexpr auto operator|(Rng&& range, Closure&& closure) {
    return pipeline_result<std::decay_t<Rng>, std::decay_t<Closure>>(
        std::forward<Rng>(range), std::forward<Closure>(closure)
    );
}

} // namespace atom::utils
