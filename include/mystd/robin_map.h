#pragma once
#include "allocator.h"
#include "cstddef"
#include "functional"
#include <cstddef>
#include <iterator>
#include <memory>
// implementation following Tessil/robin-map

template <typename Key, typename T, typename Hash = std::hash<Key>,
          typename Alloc = mystd::PoolAllocator<std::pair<Key, T>>>
class rmap {
private:
    using KVpair = std::pair<Key, T>;
    class bucket {
    public:
        bucket() : distance_from_ideal_{-1}, is_last_bucket_{false} {}
        bucket(std::ptrdiff_t distance_from_ideal)
            : distance_from_ideal_{distance_from_ideal},
              is_last_bucket_{false} {}
        bool empty() const { return distance_from_ideal_ == -1; }
        bool is_last_bucket() const { return is_last_bucket_; }
        void set_last_bucket() { is_last_bucket_ = true; }
        KVpair &value() {
            return *std::launder(reinterpret_cast<KVpair *>(kvpair_));
        }
        const KVpair &value() const {
            return *std::launder(reinterpret_cast<KVpair *>(kvpair_));
        }
        std::ptrdiff_t distance_from_ideal() { return distance_from_ideal_; }
        void clear() {
            if (!empty()) {
                destroy();
                distance_from_ideal_ = -1;
            }
        };

        void set_value_of_empty_bucket(std::ptrdiff_t dist_from_ideal_bucket,
                                       KVpair &kvpair) {
            ::new (kvpair_) KVpair{kvpair};
            distance_from_ideal_ = dist_from_ideal_bucket;
        }

    private:
        void destroy() { value()->~KVpair(); };
        std::ptrdiff_t distance_from_ideal_;
        bool is_last_bucket_;
        alignas(KVpair) std::byte kvpair_[sizeof(KVpair)];
    };
    struct key_select {
        Key &operator()(const KVpair &pair) { return pair.first; };
    };
    struct value_select {
        Key &operator()(const KVpair &pair) { return pair.second; };
    };

    template <bool is_const = false>
    class robin_iterator {
        friend class rmap;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type =
            std::conditional_t<is_const, const typename rmap::KVpair,
                               typename rmap::KVpair>;
        using pointer = typename rmap::KVpair *;
        using reference = typename rmap::KVpair &;
        using difference_type = std::ptrdiff_t;
        robin_iterator() = default;
        reference operator*() { return bucket_->kvpair_; };
        pointer operator->() { return std::addressof(bucket_->kvpair_); };
        robin_iterator &operator++() {
            while (true) {
                if (bucket_->is_last_bucket_) {
                    ++bucket_;
                    return *this;
                }
                ++bucket_;
                if (!bucket_->empty())
                    return *this;
            }
        };
        robin_iterator &operator++(int) {
            auto &tmp = *this;
            ++bucket_;
            return tmp;
        };

    private:
        robin_iterator(rmap::bucket *bucket_entry) : bucket_{bucket_entry} {};
        using bucket_ptr =
            std::conditional_t<is_const, const typename rmap::bucket *,
                               typename rmap::bucket *>;
        bucket_ptr bucket_;
    };

public:
    using const_iterator = const robin_iterator<true>;
    using iterator = robin_iterator<false>;
    std::pair<iterator, bool> insert(const Key &key, T value) {
        auto ihash = hash(key);
        auto ibucket = bucket_from_hash(ihash);
        std::ptrdiff_t distance = 0;
        while (distance <= ibucket.distance_from_ideal()) {
            if (key_select(ibucket.value()) == key) {
                return std::make_pair(robin_iterator(ibucket), false);
            }
            ibucket++;
            distance++;
        };
        // rehash for load?
        // steal
    };
    T find(const Key &item) { find_help(item, hash(item)); };
    void erase(const Key &item) { find_help(item, hash(item)); };
    Key *operator[](Key key);

private:
    constexpr static std::size_t SIZE = 100;
    bucket buckets_[SIZE];
    std::size_t load_factor_;
    std::size_t next_bucket(std::size_t index) {
        index++;
        if (index >= SIZE)
            return 0;
        return index;
    };
    bucket &bucket_from_hash(std::size_t hash) { return buckets_[hash % SIZE]; }
    const_iterator find_help(const Key &key, std::size_t hash) {
        auto ibucket = robin_iterator<true>(bucket_from_hash(hash));
        std::size_t distance = 0;
        while (ibucket.distance_from_ideal() == -1 &&
               distance <= ibucket.distance_from_ideal()) {
            if (key == key_select(ibucket.value())) {
                return ibucket;
            }
            ibucket++;
            distance++;
        }
    }
    void erase_from_bucket(const_iterator iter) {
        iter->clear();
        std::size_t prev = static_cast<std::size_t>(iter.bucket_ - buckets_);
        std::size_t next = next_bucket(prev);
        while (buckets_[next].distance_from_ideal() > 0) {
            std::ptrdiff_t new_distance =
                buckets_[next].dist_from_ideal_bucket() - 1;
            buckets_[prev].set_value_of_empty_bucket(new_distance,
                                                     buckets_[next].value());
            buckets_[next].clear();
            prev = next;
            next = next_bucket(next);
        };
    }
    bool rehash() {}
    std::size_t hash(Key key) { return Hash{}(key); }
    void steal() {}
};
