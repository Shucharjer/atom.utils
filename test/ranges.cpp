#include "ranges.hpp"
#include <algorithm>
#include <array>
#include <list>
#include <map>
#include <ranges>
#include <vector>
#include "core/pair.hpp"
#include "core/pipeline.hpp"
#include "output.hpp"
#include "ranges/element_view.hpp"
#include "ranges/to.hpp"

using namespace atom::utils;

template <typename First, typename Second>
using cpair = compressed_pair<First, Second>;

int main() {
    // to
    {
        std::map<int, int> map = {
            {   1,   5 },
            { 534,   2 },
            {  34, 756 },
            { 423,  87 },
            { 756,  15 }
        };
        auto vector = ranges::to<std::vector>(map | std::views::values);
        std::ranges::for_each(vector, print);
        newline();

        std::array<char, 5> array = { 'w', 'o', 'r', 'd', '\0' };
        auto string               = ranges::to<std::string>(array);
        print(string);
        newline();

        auto to_list = ranges::to<std::list>();

        auto list = array | to_list;
        std::ranges::for_each(list, print);
        newline();
    }

    // element view
    {
        std::initializer_list<std::pair<const int, int>> list = {
            {  423,   423 },
            { 4234, 23423 }
        };

        std::map<int, int> map = list;
        std::ranges::for_each(map | views::keys, print);
        newline();
        std::ranges::for_each(map | views::values, print);
        newline();

        std::vector<cpair<int, std::string>> vec;

        // this is ill-formed:
        // auto sks = vec | std::views::keys;
        // but the fellow isn't.

        auto ks = vec | views::keys;
        std::ranges::for_each(ks, print);
        newline();
        auto vs = vec | views::values;
        std::ranges::for_each(vs, print);
        newline();
    }
}
