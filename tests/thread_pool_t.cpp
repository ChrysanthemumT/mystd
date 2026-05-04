#include "mystd/thread_pool.h"
#include <cassert>
#include <iostream>

int main() {
    mystd::thread_pool<4, 1024> pool;
    std::atomic<int> counter{0};

    for (int i = 0; i < 100; i++) {
        pool.submit(
            [&counter]() { counter.fetch_add(1, std::memory_order_relaxed); });
    }

    // give threads time to finish
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(counter.load() == 100);
    std::cout << "ok\n";
}
