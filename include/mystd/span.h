#include <type_traits>
#include <utility>

namespace mystd {
template <typename U, typename T>
concept RangeColl = requires(U u) {
    { u.data() } -> std::convertible_to<T *>;
    { u.size() } -> std::convertible_to<std::size_t>;
};

template <typename T>
class span {
  public:
    span() : size_{0} {}
    span(T *begin, std::size_t size) : ptr_{begin}, size_{size} {}
    span(T *begin, T *end) {
        ptr_ = begin;
        size_ = end - begin;
    }
    template <RangeColl<T> Coll>
    span(const Coll &coll) : ptr_{coll.data()}, size_{coll.size()} {}
    template <std::size_t N>
    span(const T (&arr)[N]) : size_{N}, ptr_{arr} {}

    /*core*/
    T &operator[](std::size_t i) const { return ptr_[i]; }
    T *data() const { return ptr_; }
    std::size_t size() const { return size_; }
    bool empty() const { return !size_; }
    T *begin() const { return ptr_; }
    T *end() const { return ptr_ + size_; }

    span<T> subspan(std::size_t offset, std::size_t count) {
        return span<T>(ptr_ + offset, count);
    }

  private:
    T *ptr_;
    std::size_t size_;
};
}; // namespace mystd
