#include "mystd/lmax_disruptor.h"
#include "mystd/vector.h"
#include <cassert>
#include <iostream>
#include <thread>

int main() {
    const std::size_t CONSUMERS = 2;
    mystd::lmax_disruptor<int, 20, CONSUMERS> ld;
    std::thread producer([&]() {
        for (int i = 0; i < 200; i++) {
            if (i % 100 == 0)
                std::cout << "producer: " << i << "\n";
            int *slot = ld.get_slot();
            new (slot) int{i};
            ld.publish();
        };
        std::cout << "producer done\n";
    });
    std::vector<std::thread> consumers;
    for (int i = 0; i < CONSUMERS; i++) {
        consumers.emplace_back([&, i]() {
            for (int j = 0; j < 200; j++) {
                if (j % 100 == 0)
                    std::cout << "consumer " << i << ": " << j << "\n";
                int value;
                while (!ld.try_pop(value, i))
                    ;
                assert(value == j);
            }
            std::cout << "consumer " << i << " done\n";
        });
    }
    producer.join();
    for (auto &consumer : consumers) {
        if (consumer.joinable())
            consumer.join();
    }
    return 0;
}
