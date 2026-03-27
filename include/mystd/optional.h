#include <stdexcept>
#include <string>
namespace mystd {
struct nullopt_t {};
inline constexpr nullopt_t nullopt;
template <typename T> class optional {
  public:
    optional(T item) : item_(item), d_exists_(true) {}
    optional(nullopt_t) : d_exists_(false) {}
    optional(const optional &other) {
        item_ = other.item_;
        d_exists_ = true;
    }
    optional &operator=(const optional &other) {
        if (&other == this)
            return *this;
        item_ = other.item_;
        d_exists_ = true;
        return *this;
    }
    optional(optional &&other) {
        item_ = std::move(other.item_);
        d_exists_ = true;
        other.item_ = nullptr;
    }
    optional &operator=(optional &&other) {
        if (&other == this)
            return *this;
        item_ = std::move(other.item_);
        d_exists_ = true;
        other.item_ = nullptr;
        return *this;
    }
    /*core*/
    bool has_value() const { return d_exists_; }
    operator bool() const { return d_exists_; }
    T &value() const {
        if (d_exists_)
            return item_;
        else
            throw std::out_of_range("oops");
    }
    T &value_or(T other) {
        if (d_exists_)
            return item_;
        else
            return other;
    }
    void reset() { d_exists_ = false; }
    ~optional() {}

  private:
    bool d_exists_;
    T item_;
};
}; // namespace mystd
