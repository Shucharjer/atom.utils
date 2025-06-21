#include "thread.hpp"
#include <latch>
#include "output.hpp"
#include "thread/coroutine.hpp"
#include "thread/thread_pool.hpp"

using namespace atom::utils;

template <typename Ty>
coroutine<int> number_generator() {
    int current{};
    while (true) {
        co_yield current++;
    }
}

int main() {
    thread_pool thread_pool;

    // coroutine
    {
        auto generator = number_generator<void>();
        for (auto i = 0; i < 100; ++i) {
            print(generator.get());
        }
    }

    // enqueue & latch test
    if (false) {
        const auto task_num = 1000000;

        std::latch latch(task_num);
        for (auto i = 0; i < task_num; ++i) {
            auto fn = []() {};
            thread_pool.enqueue(fn);
        }

        latch.wait();
        print("all tasks are finished");
    }

    return 0;
}
