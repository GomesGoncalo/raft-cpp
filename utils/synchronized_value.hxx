#pragma once

#include "detail/synchronized_value.hxx"

namespace utils {
template <typename T> class synchronized_value;

template <typename F, typename... ValueTypes>
auto apply(F &&f, synchronized_value<ValueTypes> &...args)
    -> std::invoke_result_t<F, ValueTypes &...> {
  return detail::apply(
      std::forward<F>(f),
      static_cast<detail::synchronized_value_impl<ValueTypes> &>(args)...);
}

template <typename F, typename... ValueTypes>
auto apply(F &&f, const synchronized_value<ValueTypes> &...args)
    -> std::invoke_result_t<F, ValueTypes &...> {
  return detail::apply(
      std::forward<F>(f),
      static_cast<const detail::synchronized_value_impl<ValueTypes> &>(
          args)...);
}

template <typename T>
class synchronized_value : public detail::synchronized_value_impl<T> {
private:
  using BaseType = detail::synchronized_value_impl<T>;

public:
  using value_type = T;

  synchronized_value() noexcept(std::is_nothrow_default_constructible_v<T>)
      : BaseType{} {}

  template <typename U>
  synchronized_value(U &&newValue) noexcept(
      std::is_nothrow_constructible_v<T, U &&>)
      : BaseType{std::forward<U>(newValue)} {}

  template <typename... Args>
  synchronized_value(std::in_place_t, Args &&...args) noexcept(
      std::is_nothrow_constructible_v<T, Args &&...>)
      : BaseType{std::in_place_t{}, std::forward<Args>(args)...} {}

  template <typename U, typename... Args>
  synchronized_value(
      std::in_place_t, std::initializer_list<U> ilist,
      Args &&...args) noexcept(std::
                                   is_nothrow_constructible_v<
                                       T, std::initializer_list<U> &,
                                       Args &&...>)
      : BaseType{std::in_place_t{}, ilist, std::forward<Args>(args)...} {}

  ~synchronized_value() = default;
  synchronized_value(synchronized_value &&other) : BaseType(std::move(other)) {}
  synchronized_value(const synchronized_value &other) = delete;

  template <typename U>
  synchronized_value &operator=(synchronized_value<U> &&other) {
    BaseType::operator=(std::forward<synchronized_value<U>>(other));
    return *this;
  }

  synchronized_value &operator=(synchronized_value &&other) {
    BaseType::operator=(std::move(other));
    return *this;
  }

  template <typename U> synchronized_value &operator=(U &&value) {
    BaseType::operator=(std::forward<U>(value));
    return *this;
  }

  synchronized_value &operator=(const synchronized_value &) = delete;

  void swap(synchronized_value &other) { BaseType::swap(other); }

  template <typename... Args> void emplace(Args &&...args) {
    BaseType::emplace(std::forward<Args>(args)...);
  }

  template <typename U, typename... Args>
  void emplace(std::initializer_list<U> ilist, Args &&...args) {
    BaseType::emplace(ilist, std::forward<Args>(args)...);
  }

  template <typename U> T exchange(U &&newValue) {
    return BaseType::exchange(std::forward<U>(newValue));
  }

  T value() const { return BaseType::value(); }
};
template <typename T, typename = std::enable_if_t<std::is_swappable_v<T>>>
void swap(synchronized_value<T> &lhs, synchronized_value<T> &rhs) {
  lhs.swap(rhs);
}

template <typename T, typename U>
T exchange(synchronized_value<T> &obj, U &&newValue) {
  return obj.exchange(std::forward<U>(newValue));
}

template <typename T>
bool operator==(const synchronized_value<T> &obj, const T &val) {
  return apply([&val](auto &obj) { return obj == val; }, obj);
}
template <typename T>
bool operator!=(const synchronized_value<T> &obj, const T &val) {
  return apply([&val](auto &obj) { return obj != val; }, obj);
}
template <typename T>
bool operator<(const synchronized_value<T> &obj, const T &val) {
  return apply([&val](auto &obj) { return obj < val; }, obj);
}
template <typename T>
bool operator<=(const synchronized_value<T> &obj, const T &val) {
  return apply([&val](auto &obj) { return obj <= val; }, obj);
}
template <typename T>
bool operator>(const synchronized_value<T> &obj, const T &val) {
  return apply([&val](auto &obj) { return obj > val; }, obj);
}
template <typename T>
bool operator>=(const synchronized_value<T> &obj, const T &val) {
  return apply([&val](auto &obj) { return obj >= val; }, obj);
}

template <typename T>
bool operator==(const T &val, const synchronized_value<T> &obj) {
  return apply([&val](auto &obj) { return val == obj; }, obj);
}
template <typename T>
bool operator!=(const T &val, const synchronized_value<T> &obj) {
  return apply([&val](auto &obj) { return val != obj; }, obj);
}
template <typename T>
bool operator<(const T &val, const synchronized_value<T> &obj) {
  return apply([&val](auto &obj) { return val < obj; }, obj);
}
template <typename T>
bool operator<=(const T &val, const synchronized_value<T> &obj) {
  return apply([&val](auto &obj) { return val <= obj; }, obj);
}
template <typename T>
bool operator>(const T &val, const synchronized_value<T> &obj) {
  return apply([&val](auto &obj) { return val > obj; }, obj);
}
template <typename T>
bool operator>=(const T &val, const synchronized_value<T> &obj) {
  return apply([&val](auto &obj) { return val >= obj; }, obj);
}
} // namespace utils
