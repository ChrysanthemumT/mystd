#include <atomic>
#include <cstddef>
#include <utility>

// trying to adapt Dmitry Vyukov, bounded queues
namespace mystd {
template <typename T, std::size_t N = 20>
class mpmc {
  public:
    mpmc() {
        for (int i = 0; i < N; i++) {
            buffer_[i].sequence_num.store(i, std::memory_order_relaxed);
        }
    }
    bool try_pop(T &value) {
        auto tmp_tail = tail.load(std::memory_order_relaxed);
        while (true) {
            auto &slot = buffer_[tmp_tail % N];
            auto seq = slot.sequence_num.load(std::memory_order_acquire);
            std::ptrdiff_t diff =
                (std::ptrdiff_t)seq - (std::ptrdiff_t)(tmp_tail + 1);
            if (diff == 0) {
                if (tail.compare_exchange_weak(tmp_tail, tmp_tail + 1,
                                               std::memory_order_relaxed)) {
                    value = std::move(slot.data);
                    slot.sequence_num.store(tmp_tail + N,
                                            std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false;
            } else {
                tmp_tail = tail.load(std::memory_order_relaxed);
            }
        }
    }
    bool try_push(T &value) {
        auto tmp_head = head.load(std::memory_order_relaxed);
        while (true) {
            auto seq = buffer_[tmp_head % N].sequence_num.load(
                std::memory_order_acquire);
            // ptrdiff is signed, prevents wrap around for size_t
            std::ptrdiff_t diff =
                (std::ptrdiff_t)seq - (std::ptrdiff_t)tmp_head;
            if (diff == 0) {
                if (head.compare_exchange_weak(tmp_head, tmp_head + 1,
                                               std::memory_order_relaxed)) {
                    buffer_[tmp_head % N].data = std::move(value);
                    buffer_[tmp_head % N].sequence_num.store(
                        tmp_head + 1, std::memory_order_release);
                    return true;
                }
            } else if (diff < 0) {
                return false;
            } else {
                tmp_head = head.load(std::memory_order_relaxed);
            }
        }
    }

  private:
    struct Slot {
        T data;
        // verify every value seen exactly once
        std::atomic<size_t> sequence_num;
    };
    alignas(64) Slot buffer_[N];
    alignas(64) std::atomic<std::size_t> head;
    alignas(64) std::atomic<std::size_t> tail;
};
}; // namespace mystd
