#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

namespace mystd {

// fix std::copy and std::move to use uninitialized forms
template <typename t, typename Alloc = std::allocator<t>>
class vector {
public:
    vector() : size_(0), capacity_(0) {
        internal_array_ =
            std::allocator_traits<Alloc>::allocate(alloc_, capacity_);
    };
    /*constructor and assignments operators*/
    vector(std::size_t size) : size_(size), capacity_(size * 2) {
        internal_array_ =
            std::allocator_traits<Alloc>::allocate(alloc_, capacity_);
    }
    vector(const vector &other) {
        capacity_ = other.capacity_;
        size_ = other.size_;
        internal_array_ =
            std::allocator_traits<Alloc>::allocate(alloc_, capacity_);
        std::uninitialized_copy(other.internal_array_,
                                other.internal_array_ + size_, internal_array_);
    }
    vector &operator=(const vector &other) {
        if (this == &other) {
            return *this;
        }
        capacity_ = other.capacity_;
        size_ = other.size_;
        for (std::size_t i = 0; i < size_; i++)
            std::allocator_traits<Alloc>::destroy(alloc_, internal_array_ + i);
        std::allocator_traits<Alloc>::deallocate(alloc_, internal_array_,
                                                 capacity_);
        internal_array_ =
            std::allocator_traits<Alloc>::allocate(alloc_, capacity_);
        std::uninitialized_copy(other.internal_array_,
                                other.internal_array_ + size_, internal_array_);
        return *this;
    }
    vector(vector &&other) {
        capacity_ = other.capacity_;
        size_ = other.size_;
        alloc_ = other.alloc_;
        internal_array_ = std::exchange(other.internal_array_, nullptr);
        other.capacity_ = 0;
        other.size_ = 0;
        // deleteing nullptr is no op
    }
    vector &operator=(vector &&other) {
        if (this == &other) {
            return *this;
        }
        capacity_ = other.capacity_;
        size_ = other.size_;
        for (std::size_t i = 0; i < size_; i++)
            std::allocator_traits<Alloc>::destroy(alloc_, internal_array_ + i);
        std::allocator_traits<Alloc>::deallocate(alloc_, internal_array_,
                                                 capacity_);
        internal_array_ = std::exchange(other.internal_array_, nullptr);
        other.capacity_ = 0;
        other.size_ = 0;
        return *this;
    }

    vector(std::initializer_list<t> list) {
        size_ = list.size();
        capacity_ = size_ * 2;
        internal_array_ =
            std::allocator_traits<Alloc>::allocate(alloc_, capacity_);
        std::uninitialized_copy(list.begin(), list.end(), internal_array_);
    }
    /*core util*/
    t &operator[](std::size_t index) {
        // if (index < 0 || index >= size_) return nullptr; doesn't work can't
        // return nullptr for reference type, also std::vector out of bound
        // access is undefined
        return internal_array_[index];
    }
    const t &operator[](std::size_t index) const {
        // if (index < 0 || index >= size_) return nullptr; doesn't work can't
        // return nullptr for reference type, also std::vector out of bound
        // access is undefined
        return internal_array_[index];
    }
    void push_back(t &&item) {
        if (size_ == capacity_)
            grow();
        // internal_array_[size_++] = std::move(item);
        std::allocator_traits<Alloc>::construct(alloc_, internal_array_ + size_,
                                                std::move(item));
        size_++;
    }
    void push_back(const t &item) {
        if (size_ == capacity_)
            grow();
        // internal_array_[size_++] = item;
        std::allocator_traits<Alloc>::construct(alloc_, internal_array_ + size_,
                                                item);
        size_++;
    }
    void pop_back() {
        std::allocator_traits<Alloc>::destroy(alloc_,
                                              internal_array_ + size_ - 1);
        --size_;
    }

    template <typename... Args>
    void emplace_back(Args &&...args) {
        if (size_ == capacity_)
            grow();
        // uses placement new, old
        // new (internal_array_ + size_) t(std::forward<args>(args)...);
        // use std::allocator_traits
        std::allocator_traits<Alloc>::construct(alloc_, internal_array_ + size_,
                                                std::forward<Args>(args)...);
        size_++;
    }

    void resize(std::size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
        } else {
            if (new_size > capacity_)
                reserve(new_size);
            for (std::size_t i = size_; i < new_size; ++i) {
                new (internal_array_ + i) t;
            }
            size_ = new_size;
        }
    }

    void clear() {
        for (std::size_t i = 0; i < size_; i++) {
            std::allocator_traits<Alloc>::destroy(alloc_, internal_array_ + i);
        }
        size_ = 0;
    }
    void insert(std::size_t pos, const t &item) {
        if (pos == size_)
            push_back(item);
        else if (pos < size_) {
            if (size_ + 1 > capacity_)
                grow();
            std::move(internal_array_ + pos, internal_array_ + size_,
                      internal_array_ + pos + 1);
            internal_array_[pos] = item;
            size_++;
        }
    }
    void erase(std::size_t pos) {
        if (pos < size_) {
            std::move(internal_array_ + pos + 1, internal_array_ + size_,
                      internal_array_ + pos);
            size_--;
        }
    }
    void reserve(std::size_t new_capacity) {
        if (new_capacity > capacity_) {
            auto tmp =
                std::allocator_traits<Alloc>::allocate(alloc_, new_capacity);
            std::uninitialized_move(internal_array_, internal_array_ + size_,
                                    tmp);
            for (std::size_t i = 0; i < size_; i++)
                std::allocator_traits<Alloc>::destroy(alloc_,
                                                      internal_array_ + i);
            std::allocator_traits<Alloc>::deallocate(alloc_, internal_array_,
                                                     capacity_);
            capacity_ = new_capacity;
            internal_array_ = tmp;
        }
    }

    /*my getters*/
    t *begin() { return internal_array_; };
    t *end() { return internal_array_ + size_; };
    const t *begin() const { return internal_array_; };
    const t *end() const { return internal_array_ + size_; };
    t &at(std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("out of range my brother");
        }
        return internal_array_[index];
    }
    const t &at(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("out of range my brother");
        }
        return internal_array_[index];
    }
    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool is_empty() const { return size_ == 0; }
    const t &front() const { return internal_array_[0]; }
    const t &back() const { return internal_array_[size_ - 1]; }
    t &front() { return internal_array_[0]; }
    t &back() { return internal_array_[size_ - 1]; }
    ~vector() {
        for (std::size_t i = 0; i < size_; i++)
            std::allocator_traits<Alloc>::destroy(alloc_, internal_array_ + i);
        std::allocator_traits<Alloc>::deallocate(alloc_, internal_array_,
                                                 capacity_);
    }

private:
    Alloc alloc_;
    std::size_t capacity_;
    std::size_t size_;
    t *internal_array_;
    /*internal tools*/
    void grow() {
        // need to realloc
        auto new_array_ =
            std::allocator_traits<Alloc>::allocate(alloc_, (capacity_ + 1) * 2);
        // since new_array_ is just raw bytes need to uninitialized_move
        // instead of std::move algo
        std::uninitialized_move(internal_array_, internal_array_ + size_,
                                new_array_);
        for (std::size_t i = 0; i < size_; i++)
            std::allocator_traits<Alloc>::destroy(alloc_, internal_array_ + i);
        std::allocator_traits<Alloc>::deallocate(alloc_, internal_array_,
                                                 capacity_);
        capacity_ = (capacity_ + 1) * 2;
        internal_array_ = new_array_;
    }
};

} // namespace mystd
