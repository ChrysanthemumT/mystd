#include "allocator.h"
#include <cstddef>
#include <memory>

namespace mystd {
template <class T> class vector {
  public:
    vector(size_t s) : size_{s}, capacity_{s * 2}, data_{new T[s]} {}
    vector(vector &other) {}
    vector &operator=(vector &other) {}
    vector(vector &&other) {}
    vector &operator=(vector &&other) {}
    ~vector() {}

  private:
    size_t size_, capacity_;
    T *data_;
    mystd::allocator<vector> alloc;
};
} // namespace mystd
