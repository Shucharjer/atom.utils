#include "core.hpp"
#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <ranges>
#include "core/closure.hpp"
#include "core/pipeline.hpp"
#include "output.hpp"

using namespace atom::utils;

struct get_vector_fn {
    template <typename... Args>
    std::vector<int> operator()(Args&&... args) {
        return std::vector<int>(std::forward<Args>(args)...);
    }
};

struct empty_fn {
    using pipeline_tag = pipeline_tag;

    template <std::ranges::range Rng>
    Rng operator()(const Rng& range) const {
        return Rng{};
    }
};

constexpr inline empty_fn empty;

int main() {
    // pair
    {
        auto pair      = compressed_pair<int, int>{};
        auto& reversed = reverse(pair);
        auto another   = compressed_pair<int, std::string>{};
        // cause error: std::string is not trivial type.
        // auto& another_reversed = utils::reverse(another);
    }
    // pipeline
    {
        auto construct_arg = 10;
        auto get_vector    = make_closure<get_vector_fn>();
        auto result        = get_vector(construct_arg);
        assert(result.size() == construct_arg);

        std::vector vector = { 2, 3, 4, 6 };
        auto empty_vector  = vector | empty;
        assert(empty_vector.empty());

        auto closure              = empty | std::views::reverse;
        auto another_closure      = closure | std::views::reverse;
        auto another_empty_vector = vector | closure;
        assert(another_empty_vector.empty());
    }
    // closure
    {
        std::vector origin = { 2, 2, 3, 34, 2, 523, 53, 5, 346, 54, 645, 7, 4567, 56, 75 };
        auto end           = origin.cend();
        auto closure       = make_closure<get_vector_fn>(end);

        auto vec  = closure(origin.cbegin() + 2);
        auto vec2 = closure(origin.cbegin() + 3);
        auto vec3 = (origin.cbegin() + 3) | closure;
        std::ranges::for_each(vec, print);
        newline();
    }
    return 0;
}
