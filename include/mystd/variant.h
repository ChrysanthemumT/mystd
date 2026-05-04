#include <cstddef>
#include <utility>

namespace mystd {
template <typename... Args>
class variant {
  public:
    variant() : index_(0) {}
    template <typename T>
    variant(T &&item) {
        new (field_) T(std::forward<T>(item));
        index_ = index_of<T, Args...>();
    };
    variant(const variant &other) : index_(other.index_) {
        if (index_ == 0)
            return;
        using Fn_copy_t = std::size_t (*)(const void *);
        static Fn_copy_t table[]{[field_ = field_](const void *outer) {
            new (field_) Args(*reinterpret_cast<const Args *>(outer));
        }...};
        table[index_ - 1](other.field_);
    }
    variant(variant &&other) : index_(other.index_) {
        if (index_ == 0)
            return;
        using Fn_copy_t = std::size_t (*)(void *);
        static Fn_copy_t table[]{[field_ = field_](void *outer) {
            new (field_) Args(std::move(*reinterpret_cast<Args *>(outer)));
        }...};
        table[index_ - 1](other.field_);
        other.index_ = 0;
    }
    variant &operator=(const variant &other) {
        if (&other == this)
            return *this;
        index_ = other.index_;
        if (index_ == 0)
            return;
        using Fn_copy_t = std::size_t (*)(const void *);
        static Fn_copy_t table[]{[field_ = field_](const void *outer) {
            new (field_) Args(*reinterpret_cast<const Args *>(outer));
        }...};
        table[index_ - 1](other.field_);
        return *this;
    }
    variant &operator=(variant &&other) {
        if (&other == this)
            return *this;
        index_ = other.index_;
        if (index_ == 0)
            return;
        using Fn_copy_t = std::size_t (*)(void *);
        static Fn_copy_t table[]{[field_ = field_](void *outer) {
            new (field_) Args(std::move(*reinterpret_cast<Args *>(outer)));
        }...};
        table[index_ - 1](other.field_);
        other.index_ = 0;
        return *this;
    }
    /*core*/
    // used to pass a callable to use field_ value
    template <typename Visitor>
    void visit(Visitor &&v) {
        using Fn = void (*)(void *);
        static Fn table[]{[field_ = field_](void *callable) {
            callable(*reinterpret_cast<Args *>(field_));
        }...};
        table[index_ - 1](v);
    }
    ~variant() {
        using Fn = void (*)();
        static Fn table[]{[field_ = field_]() { field_->~Args(); }...};
        table[index_ - 1]();
    }

  private:
    alignas(Args...) std::byte field_[max({sizeof(Args)...})];
    std::size_t index_;
    template <typename T, typename First, typename... Rest>
    constexpr std::size_t index_of() {
        if constexpr (std::is_same_v<T, First>) {
            return 1;
        } else if constexpr (sizeof...(Rest) == 0) {
            static_assert(sizeof...(Rest) != 0, "Bruh! wrong type");
        } else {
            return 1 + index_of<T, Rest...>();
        }
    }
};
template <typename T, typename... Args>
T get(const variant<Args...> &v) {};
}; // namespace mystd
