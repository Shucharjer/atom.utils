#include "meta/sequence.hpp"
#include "meta/algorithm.hpp"
#include "meta.hpp"
#include "require.hpp"
#include <type_traits>
#include "output.hpp"
#include <vector>
#include <algorithm>

using namespace atom::utils;

int main() {
    // quick sort
    {
        using seq = sequence<int, 3, -3, 2, 2, 0, 1, 4>;
        using first_result = quick_sort_t<seq>;
        using first_answer = sequence<int, -3, 0, 1, 2, 2, 3, 4>;
        
        REQUIRES((std::is_same_v<first_result, first_answer>))

        using second_result = quick_sort_t<seq, greater>;
        using second_answer = sequence<int, 4, 3, 2, 2, 1, 0, -3>;
        REQUIRES((std::is_same_v<second_result, second_answer>))
    }

    // as list
    {
        using seq = sequence<int, 1,1,1,1,1,3,1>;
        std::vector vec = as_container_v<seq>;
        std::ranges::for_each(vec, print);
        newline();
    }
    
    return 0;
}