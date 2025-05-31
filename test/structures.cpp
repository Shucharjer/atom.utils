#include "structures.hpp"
#include <memory_resource>
#include <ranges>
#include <ranges/element_view.hpp>
#include "structures/dense_map.hpp"

using namespace atom::utils;

int main() {
    auto v = std::ranges::input_range<std::vector<int>>;
    std::pmr::unsynchronized_pool_resource pool;
    // dense_map
    {
        pmr::dense_map<size_t, int> map(&pool);
        auto values = map | views::values;

        dense_map<unsigned int, int> another{
            { 2,  4 },
            { 4,  4 },
            { 4, 56 }
        };

        const auto cmap = map;

        for (auto v : map) {}

        for (const auto v : map) {}

        std::vector<int> vec;

        for (auto i = map.begin(); i != map.end(); ++i) {}

        for (auto i = map.cbegin(); i != map.cend(); ++i) {}

        for (auto v : cmap) {}

        auto i = cmap.begin();
        for (i = cmap.begin(); i != cmap.end(); ++i) {}

        for (auto v : values) {}
    }

    // dense_set

    return 0;
}
