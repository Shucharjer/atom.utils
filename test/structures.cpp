#include "structures.hpp"
#include <ranges>
#include "structures/dense_map.hpp"

using namespace atom;

int main() {
    // dense_map
    {
        utils::dense_map<size_t, int> dense_map;
        auto values = dense_map | std::views::values;
    }

    // dense_set

    return 0;
}
