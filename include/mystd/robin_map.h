#pragma once
#include "allocator.h"
#include "cstddef"
#include "functional"
#include <cstddef>
#include <iterator>
#include <memory>
// implementation following Tessil/robin-map

namespace mystd {
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
            return *std::launder(reinterpret_cast<const KVpair *>(kvpair_));
        }
        std::ptrdiff_t distance_from_ideal() { return distance_from_ideal_; }
        void clear() {
            if (!empty()) {
                destroy();
                distance_from_ideal_ = -1;
            }
        };

        void set_value_of_empty_bucket(std::ptrdiff_t dist_from_ideal_bucket,
                                       const KVpair &kvpair) {
            ::new (kvpair_) KVpair{kvpair};
            distance_from_ideal_ = dist_from_ideal_bucket;
        }
        void swap_value(std::ptrdiff_t &other_dfi, KVpair &other_kvpair) {
            std::swap(distance_from_ideal_, other_dfi);
            std::swap(value(), other_kvpair);
        }

    private:
        void destroy() { value().~KVpair(); };
        std::ptrdiff_t distance_from_ideal_;
        bool is_last_bucket_;
        alignas(KVpair) std::byte kvpair_[sizeof(KVpair)];
    };
    struct key_select {
        const Key &operator()(const KVpair &pair) { return pair.first; };
    };
    struct value_select {
        const Key &operator()(const KVpair &pair) { return pair.second; };
    };

    template <bool is_const = false>
    class robin_iterator {
        friend class rmap;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type =
            std::conditional_t<is_const, const typename rmap::KVpair,
                               typename rmap::KVpair>;
        using pointer =
            std::conditional_t<is_const, const typename rmap::KVpair *,
                               typename rmap::KVpair *>;
        using reference =
            std::conditional_t<is_const, const typename rmap::KVpair &,
                               typename rmap::KVpair &>;
        using difference_type = std::ptrdiff_t;
        robin_iterator() = default;
        reference operator*() { return bucket_->value(); };
        pointer operator->() { return std::addressof(bucket_->value()); };
        reference operator*() const { return bucket_->value(); }
        pointer operator->() const { return std::addressof(bucket_->value()); }
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
            auto tmp = *this;
            ++(*this);
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
    rmap() { buckets_[SIZE - 1].set_last_bucket(); }
    std::pair<iterator, bool> insert(const Key &key, T value) {
        auto ihash = hash(key);
        std::size_t ibucket = bucket_from_hash(ihash);
        std::ptrdiff_t distance = 0;
        bool last_item_found = false;
        while (distance <= buckets_[ibucket].distance_from_ideal()) {
            if (key_select{}(buckets_[ibucket].value()) == key) {
                return std::make_pair(robin_iterator(&buckets_[ibucket]),
                                      false);
            }
            ibucket = next_bucket(ibucket);
            distance++;
        };
        if (ibucket == SIZE) {
            return std::make_pair(end(), false);
        }
        if (buckets_[ibucket].empty()) {
            buckets_[ibucket].set_value_of_empty_bucket(distance,
                                                        KVpair{key, value});
        } else {
            steal(ibucket, distance, KVpair{key, value});
        }
        return std::make_pair(robin_iterator(&buckets_[ibucket]), true);
    }
    std::pair<const_iterator, bool> find(const Key &item) {
        return find_help(item, hash(item));
    }
    bool erase(const Key &item) {
        auto [iter, err] = find_help(item, hash(item));
        if (!err)
            return err;
        erase_from_bucket(iter);
        return true;
    }
    T &operator[](Key key) {
        auto [iter, inserted] = insert(key, T{});
        return iter->second;
    }
    iterator begin() { return robin_iterator(&buckets_[0]); };
    iterator end() { return robin_iterator(&buckets_[SIZE]); };
    const_iterator begin() const { return robin_iterator<true>(&buckets_[0]); };
    const_iterator end() const {
        return robin_iterator<true>(&buckets_[SIZE]);
    };

private:
    Alloc alloc_;
    constexpr static std::size_t SIZE = 100;
    bucket buckets_[SIZE];
    std::size_t load_factor_;
    std::size_t next_bucket(std::size_t index) {
        index++;
        if (index >= SIZE)
            return 0;
        return index;
    };
    std::size_t bucket_from_hash(std::size_t hash) { return hash % SIZE; }
    std::pair<const_iterator, bool> find_help(const Key &key,
                                              std::size_t hash) {
        auto ibucket = bucket_from_hash(hash);
        std::size_t distance = 0;
        while (distance <= buckets_[ibucket].distance_from_ideal()) {
            if (key == key_select{}(buckets_[ibucket].value())) {
                return std::make_pair(robin_iterator<true>(&buckets_[ibucket]),
                                      true);
            }
            ibucket = next_bucket(ibucket);
            distance++;
        }
        return std::make_pair(robin_iterator<true>(&buckets_[SIZE]), false);
    }
    void erase_from_bucket(const_iterator iter) {
        iter->clear();
        std::size_t prev = static_cast<std::size_t>(iter.bucket_ - buckets_);
        std::size_t next = next_bucket(prev);
        while (buckets_[next].distance_from_ideal() > 0) {
            std::ptrdiff_t new_distance =
                buckets_[next].distance_from_ideal() - 1;
            buckets_[prev].set_value_of_empty_bucket(new_distance,
                                                     buckets_[next].value());
            buckets_[next].clear();
            prev = next;
            next = next_bucket(next);
        };
    }
    std::size_t hash(Key key) { return Hash{}(key); }
    void steal(std::size_t ibucket, std::ptrdiff_t distance, KVpair kvpair) {
        buckets_[ibucket].swap_value(distance, kvpair);
        ibucket = next_bucket(ibucket);
        distance++;
        while (!buckets_[ibucket].empty()) {
            if (distance > buckets_[ibucket].distance_from_ideal()) {
                buckets_[ibucket].swap_value(distance, kvpair);
                // add rehashing here?
            }
            distance++;
            ibucket = next_bucket(ibucket);
        }
        buckets_[ibucket].set_value_of_empty_bucket(distance, kvpair);
    }
};
}; // namespace mystd
