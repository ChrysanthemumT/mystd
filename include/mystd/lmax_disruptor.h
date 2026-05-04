#include <atomic>
#include <iostream>

// bug fixed;
// the slowest wasn't updating, because i didn't reset the min_value
// which was stuck at 0, since that is the global min,
namespace mystd {
template <typename t, std::size_t n = 20, std::size_t nconsum = 4>
class lmax_disruptor {
  public:
    lmax_disruptor() {
        for (auto &c : ccounter_) {
            c.store(0, std::memory_order_relaxed);
        }
        head_.store(0, std::memory_order_relaxed);
    }
    lmax_disruptor(lmax_disruptor &&other) = delete;
    lmax_disruptor(const lmax_disruptor &other) = delete;
    void operator=(const lmax_disruptor &other) = delete;
    void operator=(lmax_disruptor &&other) = delete;

    t *get_slot() {
        auto tmp_head = head_.load(std::memory_order_acquire);
        std::size_t curr_slowest = tmp_head;
        do {
            curr_slowest = tmp_head;
            get_slowest(curr_slowest);
            // } while (tmp_head - curr_slowest >= n - 1);
        } while ((tmp_head + 1) % n == curr_slowest % n);
        return &buffer_[tmp_head % n];
    }
    void publish() { head_.fetch_add(1, std::memory_order_release); }
    bool try_pop(t &value, std::size_t consumer_id) {
        auto tmp_counter =
            ccounter_[consumer_id].load(std::memory_order_acquire);
        auto tmp_head = head_.load(std::memory_order_acquire);
        // std::cout << tmp_counter << "\n";
        if (tmp_head == tmp_counter) {
            // std::cout << tmp_head << "\n";
            return false;
        }
        value = buffer_[tmp_counter % n];
        ccounter_[consumer_id].fetch_add(1, std::memory_order_release);
        return true;
    }
    ~lmax_disruptor() = default;

  private:
    alignas(64) t buffer_[n];
    alignas(64) std::atomic<std::size_t> ccounter_[nconsum];
    alignas(64) std::atomic<std::size_t> head_;
    void get_slowest(std::size_t &curr_slowest) {
        for (const auto &counter : ccounter_) {
            curr_slowest =
                std::min(curr_slowest, counter.load(std::memory_order_relaxed));
        }
    }
};
}; // namespace mystd
