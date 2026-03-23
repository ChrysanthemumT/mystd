#include <iostream>
namespace mystd {

template <typename T> class unique_ptr {
  public:
    /*constructors and assignment operators*/
    unique_ptr(T *item) { internal_ptr_ = item; };
    unique_ptr(const unique_ptr &other) = delete;
    unique_ptr(unique_ptr &&other) {
        internal_ptr_ = std::exchange(other.internal_ptr_, nullptr);
    };
    void operator=(const unique_ptr &other) = delete;
    unique_ptr &operator=(unique_ptr &&other) {
        if (&other == this)
            return *this;
        delete internal_ptr_;
        internal_ptr_ = std::exchange(other.internal_ptr_, nullptr);
        return *this;
    };
    /*core*/
    T *operator->() { return internal_ptr_; }
    T &operator*() { return *internal_ptr_; }
    const T *operator->() const { return internal_ptr_; }
    const T &operator*() const { return *internal_ptr_; }
    /*getters*/
    T *get() const { return internal_ptr_; }
    ~unique_ptr() { delete internal_ptr_; };

  private:
    T *internal_ptr_;
};

template <typename T> class shared_ptr {
    struct control_block {
        std::atomic<std::size_t> reference_count_;
        T *item_;
    };

  public:
    shared_ptr(T *item) {
        cb_ptr_ = new control_block;
        cb_ptr_->reference_count_ = 1;
        cb_ptr_->item_ = item;
    }
    shared_ptr(const shared_ptr &other) {
        cb_ptr_ = other.cb_ptr_;
        cb_ptr_->reference_count_.fetch_add(1, std::memory_order_relaxed);
    }
    shared_ptr(shared_ptr &&other) {
        cb_ptr_ = std::exchange(other.cb_ptr_, nullptr);
    }
    shared_ptr &operator=(const shared_ptr &other) {
        if (&other == this)
            return *this;
        release();
        cb_ptr_ = other.cb_ptr_;
        cb_ptr_->reference_count_.fetch_add(1, std::memory_order_relaxed);
        return *this;
    }
    shared_ptr &operator=(shared_ptr &&other) {
        if (&other == this)
            return *this;
        release();
        cb_ptr_ = std::exchange(other.cb_ptr_, nullptr);
        return *this;
    }
    /*core*/
    T *operator->() { return cb_ptr_->item_; }
    T &operator*() { return *cb_ptr_->item_; }
    const T *operator->() const { return cb_ptr_->item_; }
    const T &operator*() const { return *cb_ptr_->item_; }
    T *get() { return cb_ptr_->item_; }
    const T *get() const { return cb_ptr_->item_; }
    ~shared_ptr() {
        if (!cb_ptr_)
            return;
        if (cb_ptr_->reference_count_.fetch_sub(1, std::memory_order_acq_rel) ==
            1) {
            delete cb_ptr_->item_;
            delete cb_ptr_;
        }
    }

  private:
    struct control_block *cb_ptr_;
    void release() {
        if (!cb_ptr_)
            return;
        if (cb_ptr_->reference_count_.fetch_sub(1, std::memory_order_acq_rel) ==
            1) {
            delete cb_ptr_->item_;
            delete cb_ptr_;
        }
        cb_ptr_ = nullptr;
    };
};

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args &&...args) {
    return unique_ptr<T>(new T(std::forward<Args>(args)...));
};

template <typename T, typename... Args>
shared_ptr<T> make_unique(Args... args) {
    return shared_ptr<T>(new T(std::forward<Args>(args)...));
}
}; // namespace mystd
