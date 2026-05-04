#include "mystd/mpmc.h"
#include <atomic>
#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    mystd::mpmc<int, 1024> queue;
    constexpr int TOTAL = 10000;
    constexpr int PRODUCERS = 2;
    constexpr int CONSUMERS = 2;
    constexpr int PER_PRODUCER = TOTAL / PRODUCERS;

    std::atomic<int> produced{0};
    std::atomic<int> consumed{0};
    std::vector<std::atomic<int>> seen(TOTAL);
    for (auto &s : seen)
        s.store(0);

    std::vector<std::thread> producers;
    for (int p = 0; p < PRODUCERS; p++) {
        producers.emplace_back([&, p]() {
            int start = p * PER_PRODUCER;
            int end = start + PER_PRODUCER;
            for (int i = start; i < end; i++) {
                int value = i;
                while (!queue.try_push(std::move(value)))
                    ;
                produced.fetch_add(1);
            }
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < CONSUMERS; c++) {
        consumers.emplace_back([&]() {
            while (consumed.load() < TOTAL) {
                int val;
                if (queue.try_pop(val)) {
                    seen[val].fetch_add(1);
                    consumed.fetch_add(1);
                }
            }
        });
    }

    for (auto &t : producers)
        t.join();
    for (auto &t : consumers)
        t.join();

    for (int i = 0; i < TOTAL; i++) {
        assert(seen[i].load() == 1 && "value missing or duplicated");
    }

    std::cout << "produced: " << produced.load() << "\n";
    std::cout << "consumed: " << consumed.load() << "\n";
    std::cout << "done!!!!!\n";
}
