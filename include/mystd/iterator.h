namespace mystd {
template <typename T> class iterator {
  public:
    iterator();
    ~iterator() = default;

  private:
    T *begin_, end_;
};
} // namespace mystd
