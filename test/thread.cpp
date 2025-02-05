#include "thread.hpp"
#include <latch>
#include "output.hpp"
#include "thread/thread_pool.hpp"

using namespace atom;

int main() {
    {
        utils::thread_pool thread_pool;
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
