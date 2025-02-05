#include "ranges.hpp"
#include <algorithm>
#include <array>
#include <map>
#include <vector>
#include "output.hpp"
#include "ranges/to.hpp"

using namespace atom;

int main() {
    std::map<int, int> map = {
        {   1,   5 },
        { 534,   2 },
        {  34, 756 },
        { 423,  87 },
        { 756,  15 }
    };
    auto vector = utils::ranges::to<std::vector>(map | std::views::values);
    std::ranges::for_each(vector, print);
    newline();

    std::array<char, 5> array = { 'w', 'o', 'r', 'd', '\0' };
    auto string               = utils::ranges::to<std::string>(array);
    print(string);
    newline();
}
