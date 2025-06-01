#include "memory.hpp"
#include <chrono>
#include <format>
#include <thread>
#include <unordered_map>
#include <vector>
#include "memory/allocator.hpp"
#include "memory/pool.hpp"
#include "memory/storage.hpp"
#include "misc/timer.hpp"
#include "output.hpp"

using namespace atom;
using namespace atom::utils;

int main() {
    // standard_allocator
    {
        auto allocator = utils::standard_allocator<int>{};

        // unique_storage
        {
            utils::unique_storage<int, decltype(allocator)> storage{ allocator };
            storage = 114514;
            print(*storage.get());
            newline();
        }

        // shared_storage
        {
            utils::shared_storage<int, decltype(allocator)> storage{ allocator };
            print("has value:");
            print(static_cast<bool>(storage));
            newline();
            storage = 114514;
            print("has value:");
            print(static_cast<bool>(storage));
            newline();
        }

        // vector
        { std::vector<int, decltype(allocator)> storage{ allocator }; }
    }

    // allocator
    {
        auto pool      = utils::synchronized_pool{};
        auto allocator = utils::allocator<int, utils::synchronized_pool>{ pool };

        // in std::unordered_map
        {
            using allocator_t =
                utils::allocator<std::pair<const int, int>, utils::synchronized_pool>;
            std::unordered_map<int, int, std::hash<int>, std::equal_to<int>, allocator_t> map(
                allocator_t{ allocator });
        }
    }

    // builtin_allocator
    // {
    //     utils::unique_storage<int, utils::builtin_storage_allocator<int>> storage;
    //     storage = 114514;
    //     print("value in storage:");
    //     print(*storage.get());
    //     newline();
    // }

    return 0;
}
