#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>

namespace mystd {

template <typename T> class vector {
  public:
    vector() : size_(0), capacity_(0) {};
    /*constructor and assignments operators*/
    vector(std::size_t size) : size_(size), capacity_(size * 2) {
        internal_array_ = new T[capacity_];
    }
    vector(const vector &other) {
        capacity_ = other.capacity_;
        size_ = other.size_;
        internal_array_ = new T[capacity_];
        std::copy(other.internal_array_, other.internal_array_ + size_,
                  internal_array_);
    }
    vector &operator=(const vector &other) {
        if (this == &other) {
            return *this;
        }
        capacity_ = other.capacity_;
        size_ = other.size_;
        delete[] internal_array_;
        internal_array_ = new T[capacity_];
        std::copy(other.internal_array_, other.internal_array_ + size_,
                  internal_array_);
        return *this;
    }
    vector(vector &&other) {
        capacity_ = other.capacity_;
        size_ = other.size_;
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
        delete[] internal_array_;
        internal_array_ = std::exchange(other.internal_array_, nullptr);
        other.capacity_ = 0;
        other.size_ = 0;
        return *this;
    }

    vector(std::initializer_list<T> list) {
        size_ = list.size();
        capacity_ = size_ * 2;
        internal_array_ = new T[capacity_];
        std::copy(list.begin(), list.end(), internal_array_);
    }
    /*core util*/
    T &operator[](std::size_t index) {
        // if (index < 0 || index >= size_) return nullptr; doesn't work can't
        // return nullptr for reference type, also std::vector out of bound
        // access is undefined
        return internal_array_[index];
    }
    const T &operator[](std::size_t index) const {
        // if (index < 0 || index >= size_) return nullptr; doesn't work can't
        // return nullptr for reference type, also std::vector out of bound
        // access is undefined
        return internal_array_[index];
    }
    void push_back(T &&item) {
        if (size_ == capacity_)
            grow();
        internal_array_[size_++] = std::move(item);
    }
    void push_back(const T &item) {
        if (size_ == capacity_)
            grow();
        internal_array_[size_++] = item;
    }
    void pop_back() { --size_; }

    template <typename... Args> void emplace_back(Args &&...args) {
        if (size_ == capacity_)
            grow();
        // uses placement new
        new (internal_array_ + size_) T(std::forward<Args>(args)...);
        size_++;
    }

    void resize(std::size_t new_size) {
        if (new_size < size_) {
            size_ = new_size;
        } else {
            if (new_size > capacity_)
                reserve(new_size);
            for (std::size_t i = size_; i < new_size; ++i) {
                new (internal_array_ + i) T;
            }
            size_ = new_size;
        }
    }

    void clear() { size_ = 0; }
    void insert(std::size_t pos, const T &item) {
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
            T *tmp = new T[new_capacity];
            std::move(internal_array_, internal_array_ + size_, tmp);
            capacity_ = new_capacity;
            delete[] internal_array_;
            internal_array_ = tmp;
        }
    }

    /*my getters*/
    T *begin() { return internal_array_; };
    T *end() { return internal_array_ + size_; };
    const T *begin() const { return internal_array_; };
    const T *end() const { return internal_array_ + size_; };
    T &at(std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("out of range my brother");
        }
        return internal_array_[index];
    }
    const T &at(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("out of range my brother");
        }
        return internal_array_[index];
    }
    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool is_empty() const { return size_ == 0; }
    const T &front() const { return internal_array_[0]; }
    const T &back() const { return internal_array_[size_ - 1]; }
    T &front() { return internal_array_[0]; }
    T &back() { return internal_array_[size_ - 1]; }
    ~vector() { delete[] internal_array_; }

  private:
    std::size_t capacity_;
    std::size_t size_;
    T *internal_array_;
    /*internal tools*/
    void grow() {
        // need to realloc
        T *new_array_ = new T[(capacity_ + 1) * 2];
        capacity_ = (capacity_ + 1) * 2;
        std::move(internal_array_, internal_array_ + size_, new_array_);
        delete[] internal_array_;
        internal_array_ = new_array_;
    }
};

} // namespace mystd
