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
    using propagate_on_container_move_assignment = std::true_type;
    PoolAllocator() {
        node_list_ = ::new Node_List_{};
        create_new(node_list_);
        free_list_ = node_list_->node;
    }
    ~PoolAllocator() {
        auto curr = node_list_;
        while (curr) {
            auto next = curr->next;
            delete[] curr->starting_add;
            delete curr;
            curr = next;
        }
    }
    template <typename... Args>
    T *construct(Args &&...args) {
        T *alloc = allocate();
        ::new (alloc) T(std::forward<Args>(args)...);
        return alloc;
    }
    T *allocate() {
        auto curr_block = node_list_;
        while (curr_block->next && free_list_ == nullptr) {
            curr_block = curr_block->next;
            free_list_ = curr_block->node;
        }
        if (free_list_ == nullptr) {
            auto new_block = new Node_List_{};
            create_new(new_block);
            curr_block->next = new_block;
            free_list_ = new_block->node;
        }
        T *alloc = reinterpret_cast<T *>(free_list_);
        free_list_ = free_list_->next;
        return alloc;
    }
    // these can be maintained by allocator traits, just testing
    void destroy(T *alloc) {
        alloc->~T();
        deallocate(alloc);
    }
    void deallocate(T *alloc) {
        auto curr_block = node_list_;
        auto alloc_bytes = reinterpret_cast<std::byte *>(alloc);
        while (
            !(alloc_bytes >= curr_block->starting_add) ||
            !(alloc_bytes < curr_block->starting_add + DPOOLSIZE * CHUNKSIZE)) {
            curr_block = curr_block->next;
        }
        reinterpret_cast<Node_ *>(alloc)->next = curr_block->node;
        curr_block->node = reinterpret_cast<Node_ *>(alloc);
        free_list_ = reinterpret_cast<Node_ *>(curr_block->node);
    }
    static Alloc select_on_container_copy_construction(const Alloc &alloc) {
        return alloc;
    }

private:
    struct Node_ {
        Node_ *next;
    };
    struct Node_List_ {
        std::byte *starting_add;
        Node_ *node;
        Node_List_ *next;
    };
    static constexpr std::size_t CHUNKSIZE =
        sizeof(T) >= sizeof(Node_ *) ? sizeof(T) : sizeof(Node_ *);
    // alignas(T) std::byte buffer_[DPOOLSIZE * CHUNKSIZE];
    Node_ *free_list_;
    Node_List_ *node_list_;
    void create_new(Node_List_ *node_list) {
        auto buffer_ =
            new (std::align_val_t{alignof(T)}) std::byte[DPOOLSIZE * CHUNKSIZE];
        auto curr = buffer_;
        for (int i = 0; i < DPOOLSIZE - 1; ++i) {
            reinterpret_cast<Node_ *>(curr)->next =
                reinterpret_cast<Node_ *>(curr + CHUNKSIZE);
            curr = reinterpret_cast<std::byte *>(
                reinterpret_cast<Node_ *>(curr)->next);
        }
        reinterpret_cast<Node_ *>(curr)->next = nullptr;
        node_list->node = reinterpret_cast<Node_ *>(buffer_);
        node_list->starting_add = buffer_;
    }
};
} // namespace mystd
