#include <stdexcept>
#include <utility>

namespace mystd {
/*lets do chaining use hashable concept*/
/*work on custom allocator for begin and end*/
template <typename Key, typename Value> class chash_map {
    struct node_chain {
        Key key;
        Value value;
        node_chain *next;
    };

  public:
    chash_map(std::size_t size) : bucket_size_(size) {
        num_item_ = 0;
        map_ = new node_chain
            *[bucket_size_](); // () zero initialized ptr, unitialised
        // ptr will cause trouble;
    }
    /*core*/
    Value &operator[](Key key) {
        if (static_cast<float>(num_item_) / static_cast<float>(bucket_size_) >
            0.75)
            resize();
        std::size_t idx = mhash(key);
        node_chain *item = map_[idx];
        while (item && item->key != key) {
            item = item->next;
        }
        if (item)
            return item->value;
        node_chain *tmp = new node_chain{key, Value{}, map_[idx]};
        map_[idx] = tmp;
        num_item_++;
        return tmp->value;
    }
    node_chain *find(Key key) {
        std::size_t idx = mhash(key);
        node_chain *item = map_[idx];
        while (item && item->key != key) {
            item = item->next;
        }
        return item;
    }
    void insert(Key key, Value value) {
        std::size_t idx = mhash(key);
        node_chain *item = map_[idx];
        while (item && item->key != key) {
            item = item->next;
        }
        if (item)
            return;
        map_[idx] = new node_chain{key, value, map_[idx]};
    }
    node_chain **begin() { return map_; }
    node_chain **end() { return map_ + bucket_size_; }
    const node_chain **begin() const { return map_; }
    const node_chain **end() const { return map_ + bucket_size_; }
    ~chash_map() { delete[] map_; }

  private:
    std::size_t bucket_size_;
    std::size_t num_item_;
    node_chain **map_;
    // std::hash is a functor, struct obj that is callable
    std::size_t mhash(Key key) { return std::hash<Key>{}(key) % bucket_size_; };
    void resize() {
        node_chain *tmp = new node_chain *[bucket_size_ * 2];
        for (std::size_t i = 0; i < bucket_size_; ++i) {
            // rehash;
            node_chain *curr = map_[i];
            while (curr) {
                std::size_t idx = mhash(curr->key);
                tmp[idx] = new node_chain{curr->key, curr->value, tmp[idx]};
                curr = curr->next;
            }
        }
        bucket_size_ *= 2;
        delete[] map_;
        map_ = tmp;
    }
};

/* open addressing */
template <typename Key, typename Value> class oahash_map {
  public:
  private:
};
}; // namespace mystd
