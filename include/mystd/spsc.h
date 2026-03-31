#include <atomic>
#include <utility>

namespace mystd {
template <typename T, std::size_t N = 20>
class spsc {
  public:
    spsc() {}
    T *pop() {
        auto head_tmp = head_.load(std::memory_order_acquire);
        auto tail_tmp = tail_.load(std::memory_order_relaxed);
        while (head_tmp == tail_tmp)
            head_tmp = head_.load(std::memory_order_acquire);
        T *item = reinterpret_cast<T *>(buffer_ + tail_tmp * sizeof(T));
        std::size_t new_tail = (tail_tmp + 1) % N;
        tail_.store(new_tail, std::memory_order_release);
        return item;
    }
    bool try_pop(T &out) {
        auto head_tmp = head_.load(std::memory_order_acquire);
        auto tail_tmp = tail_.load(std::memory_order_relaxed);
        if (head_tmp == tail_tmp)
            return false;
        out = reinterpret_cast<T *>(buffer_ + tail_tmp * sizeof(T));
        std::size_t new_tail = (tail_tmp + 1) % N;
        tail_.store(new_tail, std::memory_order_release);
        return true;
    }
    void push(T &&item) {
        auto tail_tmp = tail_.load(std::memory_order_acquire);
        auto head_tmp = head_.load(std::memory_order_relaxed);
        while ((head_tmp + 1) % N == tail_tmp)
            tail_tmp = tail_.load(std::memory_order_acquire);
        ;
        ::new (buffer_ + head_tmp * sizeof(T)) T(std::move(item));
        std::size_t head_new = (head_tmp + 1) % N;
        head_.store(head_new, std::memory_order_release);
    }
    void try_push(T &&item) {
        auto tail_tmp = tail_.load(std::memory_order_acquire);
        auto head_tmp = head_.load(std::memory_order_relaxed);
        if ((head_tmp + 1) % N == tail_tmp)
            return;
        ::new (buffer_ + head_tmp * sizeof(T)) T(std::move(item));
        std::size_t head_new = (head_tmp + 1) % N;
        head_.store(head_new, std::memory_order_release);
    }

  private:
    // alignas(64) prevents false sharing and cache ping ponging of atomics
    alignas(64) std::atomic<std::size_t> head_ = 0;
    alignas(64) std::atomic<std::size_t> tail_ = 0;
    alignas(T) std::byte buffer_[N * sizeof(T)];
};
} // namespace mystd
