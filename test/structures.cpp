#include "structures.hpp"
#include <ranges>
#include "structures/dense_map.hpp"

using namespace atom;

int main() {
    auto v = std::ranges::input_range<std::vector<int>>;
    // dense_map
    {
        utils::dense_map<size_t, int> dense_map;
        auto values = dense_map | std::views::values;

        const auto map = dense_map;

        for (auto v : dense_map) {}

        for (const auto v : dense_map) {}

        std::vector<int> vec;

        for (auto i = dense_map.begin(); i != dense_map.end(); ++i) {}

        for (auto i = dense_map.cbegin(); i != dense_map.cend(); ++i) {}

        for (auto v : map) {}

        auto i = map.begin();
        for (i = map.begin(); i != map.end(); ++i) {}

        for (auto v : values) {}
    }

    // dense_set

    return 0;
}
