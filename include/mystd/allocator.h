#include <exception>
#include <type_traits>
#include <utility>

namespace mystd {
// work on propagating on move, copy, swap
// might work on nesting
template <typename T, std::size_t DPOOLSIZE = 40>
class PoolAllocator {
  public:
    // this is important for alloc_traits
    using value_type = T;
    using Alloc = PoolAllocator<T>;
    // using propagate_on_container_move_assignment = true_type;
    PoolAllocator() {
        auto curr = buffer_;
        for (int i = 0; i < DPOOLSIZE - 1; ++i) {
            reinterpret_cast<Node_ *>(curr)->next =
                reinterpret_cast<Node_ *>(curr + CHUNKSIZE);
            curr = reinterpret_cast<std::byte *>(
                reinterpret_cast<Node_ *>(curr)->next);
        }
        reinterpret_cast<Node_ *>(curr)->next = nullptr;
        free_list_ = reinterpret_cast<Node_ *>(buffer_);
    }
    template <typename... Args>
    T *construct(Args &&...args) {
        T *alloc = allocate();
        ::new (alloc) T(std::forward<Args>(args)...);
        return alloc;
    }
    T *allocate() {
        if (free_list_ == nullptr)
            throw std::bad_alloc();
        T *alloc = reinterpret_cast<T *>(free_list_);
        free_list_ = free_list_->next;
        return alloc;
    }
    // these can be maintained by allocator traits
    void destroy(T *alloc) {
        alloc->~T();
        deallocate(alloc);
    }
    void deallocate(T *alloc) {
        reinterpret_cast<Node_ *>(alloc)->next = free_list_;
        free_list_ = reinterpret_cast<Node_ *>(alloc);
    }
    static Alloc select_on_container_copy_construction(const Alloc &alloc) {
        return alloc;
    }

  private:
    struct Node_ {
        Node_ *next;
    };
    static constexpr std::size_t CHUNKSIZE =
        sizeof(T) >= sizeof(Node_ *) ? sizeof(T) : sizeof(Node_ *);
    alignas(T) std::byte buffer_[DPOOLSIZE * CHUNKSIZE];
    Node_ *free_list_;
};
} // namespace mystd
