#include "mpmc.h"
#include <thread>
#include <utility>

namespace mystd {
template <std::size_t NTHREADS = 20, std::size_t NTASKS = 40>
class thread_pool {
  public:
    using Callable = std::function<void()>;
    thread_pool() {
        for (int i = 0; i < NTHREADS; i++) {
            workers_[i] = std::thread([this]() {
                Callable task;
                for (;;) {
                    if (stop_.load(std::memory_order_relaxed))
                        return;
                    if (task_queue_.try_pop(task))
                        task();
                }
            });
        }
    }
    thread_pool(const thread_pool &) = delete;
    thread_pool &operator=(const thread_pool &) = delete;
    thread_pool(thread_pool &&) = delete;
    thread_pool &operator=(thread_pool &&) = delete;
    void submit(Callable func) {
        while (!task_queue_.try_push(std::move(func)))
            ;
    }
    ~thread_pool() {
        stop_.store(true, std::memory_order_relaxed);
        for (int i = 0; i < NTHREADS; i++) {
            if (workers_[i].joinable())
                workers_[i].join();
        }
    }

  private:
    mystd::mpmc<Callable, NTASKS> task_queue_;
    std::thread workers_[NTHREADS];
    std::atomic<bool> stop_;
};
}; // namespace mystd
