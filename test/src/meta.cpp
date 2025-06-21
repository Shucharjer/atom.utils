#include "meta.hpp"
#include <algorithm>
#include <type_traits>
#include <vector>
#include "core.hpp"
#include "meta/algorithm.hpp"
#include "meta/sequence.hpp"
#include "output.hpp"
#include "require.hpp"


using namespace atom::utils;

void foo(int) {}
constexpr void cfoo(int) {}

int main() {
    // constexpr
    {
        auto lambda = [](const std::string&) { return 1; };
        // the follow line shows the lamdba is constexpr
        constexpr auto v = lambda(std::string{});
        REQUIRES(is_constexpr<lambda>())
        REQUIRES(is_constexpr<&decltype(lambda)::operator()>())
        REQUIRES(is_constexpr<&cfoo>())
        REQUIRES_FALSE(is_constexpr<&foo>())
    }

    // quick sort
    // BUG: implicit instantiation of undefined template.
    /*{
        using seq          = sequence<int>;
        using first_result = typename quick_sort<seq>::type;
        using first_answer = sequence<int, -3, 0, 1, 2, 2, 3, 4>;

        REQUIRES((std::is_same_v<first_result, first_answer>))

        using second_result = quick_sort_t<seq, greater>;
        using second_answer = sequence<int, 4, 3, 2, 2, 1, 0, -3>;
        REQUIRES((std::is_same_v<second_result, second_answer>))
    }

    // as list
    {
        using seq       = sequence<int, 1, 1, 1, 1, 1, 3, 1>;
        std::vector vec = as_container_v<seq>;
        std::ranges::for_each(vec, print);
        newline();
    }*/

    return 0;
}
