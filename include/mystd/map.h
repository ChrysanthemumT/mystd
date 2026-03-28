#include <stdexcept>
#include <utility>

namespace mystd {
template <typename T>
concept Hashable = requires(T key) { std::hash<T>{}(key); };

/*lets do chaining use hashable concept*/
/*work on custom allocator for begin and end*/
template <Hashable Key, typename Value>
class chash_map {
    struct node_chain {
        Key key;
        Value value;
        node_chain *next;
    };

  public:
    chash_map(std::size_t size) : map_size_(size) {
        num_item_ = 0;
        map_ = new node_chain
            *[map_size_](); // () zero initialized ptr, unitialised
        // ptr will cause trouble;
    }
    /*core*/
    Value &operator[](Key key) {
        if (static_cast<float>(num_item_) / static_cast<float>(map_size_) >
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
    node_chain **end() { return map_ + map_size_; }
    const node_chain **begin() const { return map_; }
    const node_chain **end() const { return map_ + map_size_; }
    ~chash_map() { delete[] map_; }

  private:
    std::size_t map_size_;
    std::size_t num_item_;
    node_chain **map_;
    // std::hash is a functor, struct obj that is callable
    std::size_t mhash(Key key) { return std::hash<Key>{}(key) % map_size_; };
    void resize() {
        node_chain *tmp = new node_chain *[map_size_ * 2];
        for (std::size_t i = 0; i < map_size_; ++i) {
            // rehash;
            node_chain *curr = map_[i];
            while (curr) {
                std::size_t idx = mhash(curr->key);
                tmp[idx] = new node_chain{curr->key, curr->value, tmp[idx]};
                curr = curr->next;
            }
        }
        map_size_ *= 2;
        delete[] map_;
        map_ = tmp;
    }
};

inline std::size_t INIT_SIZE = 20;
/* open addressing */
template <Hashable Key, typename Value>
class oahash_map {
  public:
    oahash_map() {
        map_size_ = INIT_SIZE;
        num_items_ = 0;
        map_ = new node_[map_size_];
    }
    oahash_map(std::size_t map_size) : map_size_{map_size} {
        num_items_ = 0;
        map_ = new node_[map_size_];
    }
    /*core*/
    Value &operator[](Key key) {
        std::size_t idx = mhash(key);
        if (map_[idx] == nullptr)
            new (map_[idx]) node_{key, 0, node_::State::ACT};
        else if (map_[idx]->key_ != key)
            probe(&idx, key);
        return map_[idx]->value;
    }
    ~oahash_map() {}

  private:
    struct node_ {
        Key key_;
        Value value_;
        enum class State { NIL, ACT, ZOMB } state = State::NIL;
    };
    std::size_t map_size_;
    std::size_t num_items_;
    std::size_t mhash(Key key) { return std::hash<Key>{}(key) % map_size_; }
    void probe(std::size_t *idx, Key key) {};
    node_ *map_;
};
}; // namespace mystd
