#include "core.hpp"
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <ranges>
#include "core/closure.hpp"
#include "output.hpp"

using namespace atom;

struct get_vector_fn {
    template <typename... Args>
    std::vector<int> operator()(Args&&... args) {
        return std::vector<int>(std::forward<Args>(args)...);
    }
};

struct empty_view : utils::pipeline_base<empty_view> {
    template <std::ranges::range Rng>
    Rng operator()(const Rng& range) const {
        return Rng{};
    }
};

constexpr inline empty_view empty;

int main() {
    auto construct_arg = 10;
    auto get_vector    = utils::make_closure<get_vector_fn>();
    auto result        = get_vector(construct_arg);
    assert(result.size() == construct_arg);

    std::vector vector = { 2, 3, 4, 6 };
    auto empty_vector  = vector | empty;
    assert(empty_vector.empty());

    auto closure              = empty | std::views::reverse;
    auto another_empty_vector = vector | closure;
    assert(another_empty_vector.empty());
    return 0;
}
