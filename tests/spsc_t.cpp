#include "mystd/spsc.h"
#include <cassert>
#include <iostream>
#include <thread>

int main() {
    mystd::spsc<int, 1024> queue;
    constexpr int COUNT = 10000;

    std::thread producer([&]() {
        for (int i = 0; i < COUNT; i++)
            queue.push(i);
    });

    std::thread consumer([&]() {
        int expected = 0;
        while (expected < COUNT) {
            int val;
            if (queue.try_pop(val)) {
                assert(val == expected);
                expected++;
            }
        }
    });

    producer.join();
    consumer.join();
    std::cout << "ok\n";
}
