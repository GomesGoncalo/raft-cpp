#pragma once

#include <functional>
#include <initializer_list>
#include <mutex>
#include <type_traits>
#include <utility>

namespace detail {

template <typename T> class synchronized_value_impl;

template <typename F, typename... ValueTypes>
auto apply(F &&f, synchronized_value_impl<ValueTypes> &...args)
    -> std::invoke_result_t<F, ValueTypes &...>;

template <typename F, typename... ValueTypes>
auto apply(F &&f, const synchronized_value_impl<ValueTypes> &...args)
    -> std::invoke_result_t<F, ValueTypes &...>;

template <typename T> class synchronized_value_impl {
private:
  static_assert(std::is_copy_constructible_v<T> ||
                    std::is_move_constructible_v<T>,
                "T must be either copy and/or move constructible.");

public:
  synchronized_value_impl() noexcept(std::is_nothrow_default_constructible_v<T>)
      : val{} {
    static_assert(std::is_default_constructible_v<T>,
                  "T is not default constructable");
  }

  template <
      typename U,
      std::enable_if_t<
          std::is_constructible_v<T, U &&> &&
              !std::is_same_v<std::decay_t<U>, std::in_place_t> &&
              !std::is_same_v<std::decay_t<U>, synchronized_value_impl<T>>,
          bool> = true>
  synchronized_value_impl(U &&value) noexcept(
      std::is_nothrow_constructible_v<T, U &&>)
      : val{std::forward<U>(value)} {}

  template <
      typename... Args,
      std::enable_if_t<std::is_constructible_v<T, Args &&...>, bool> = true>
  synchronized_value_impl(std::in_place_t, Args &&...args) noexcept(
      std::is_nothrow_constructible_v<T, Args &&...>)
      : val{std::forward<Args>(args)...} {}

  template <typename U, typename... Args,
            std::enable_if_t<std::is_constructible_v<
                                 T, std::initializer_list<U> &, Args &&...>,
                             bool> = true>
  synchronized_value_impl(
      std::in_place_t, std::initializer_list<U> ilist,
      Args &&...args) noexcept(std::
                                   is_nothrow_constructible_v<
                                       T, std::initializer_list<U> &,
                                       Args &&...>)
      : val{ilist, std::forward<Args>(args)...} {}

  ~synchronized_value_impl() = default;

  template <std::enable_if_t<std::is_move_constructible_v<T>, bool> = true>
  synchronized_value_impl(synchronized_value_impl &&other)
      : val{detail::apply(
            [](auto &otherVal) -> T { return std::move(otherVal); }, other)} {}

  template <
      typename U,
      std::enable_if_t<
          std::is_constructible_v<T, U &&> &&
              (!std::is_same_v<std::decay_t<T>, bool> &&
               (!std::is_constructible_v<T, synchronized_value_impl<U> &> &&
                !std::is_constructible_v<T,
                                         const synchronized_value_impl<U> &> &&
                !std::is_constructible_v<T, synchronized_value_impl<U> &&> &&
                !std::is_constructible_v<T,
                                         const synchronized_value_impl<U> &&> &&
                !std::is_convertible_v<synchronized_value_impl<U> &, T> &&
                !std::is_convertible_v<const synchronized_value_impl<U> &, T> &&
                !std::is_convertible_v<synchronized_value_impl<U> &&, T> &&
                !std::is_convertible_v<const synchronized_value_impl<U> &&,
                                       T>)),
          bool> = true>
  synchronized_value_impl(synchronized_value_impl<U> &&other)
      : val{detail::apply(
            [](auto &otherVal) -> T { return std::move(otherVal); }, other)} {}

  synchronized_value_impl(const synchronized_value_impl &) = delete;

  template <std::enable_if_t<std::is_move_constructible_v<T> ||
                                 std::is_move_assignable_v<T>,
                             bool> = true>
  synchronized_value_impl &operator=(synchronized_value_impl &&other) {
    detail::apply(
        [](auto &val, auto &otherVal) -> void { val = std::move(otherVal); },
        *this, other);

    return *this;
  }

  template <typename U,
            std::enable_if_t<
                std::is_constructible_v<T, U &&> &&
                    !std::is_same_v<std::decay_t<U>, synchronized_value_impl>,
                bool> = true>
  synchronized_value_impl &operator=(U &&newValue) {
    detail::apply(
        [&newValue](auto &val) -> void { val = std::forward<U>(newValue); },
        *this);

    return *this;
  }

  synchronized_value_impl &operator=(const synchronized_value_impl &) = delete;

  void swap(synchronized_value_impl &other) {
    static_assert(std::is_swappable_v<T>, "T is not a swappable type");

    detail::apply(
        [](auto &val, auto &otherVal) -> void {
          using std::swap;
          swap(val, otherVal);
        },
        *this, other);
  }

  template <typename... Args> void emplace(Args &&...args) {
    static_assert(std::is_constructible_v<T, Args &&...>,
                  "T is not constructible from Args&&...");

    detail::apply(
        [&args...](auto &val) -> void { val = T{std::forward<Args>(args)...}; },
        *this);
  }

  template <typename U, typename... Args>
  void emplace(std::initializer_list<U> ilist, Args &&...args) {
    static_assert(
        std::is_constructible_v<T, std::initializer_list<U> &, Args &&...>,
        "T is not constructible from initializer_list and Args&&...");

    detail::apply(
        [&ilist, &args...](auto &val) -> void {
          val = T{ilist, std::forward<Args>(args)...};
        },
        *this);
  }

  template <typename U> T exchange(U &&newValue) {
    static_assert(std::is_move_constructible_v<T>,
                  "T is not move constructible from U");

    return detail::apply(
        [&newValue](auto &val) -> T {
          return std::exchange(val, std::forward<U>(newValue));
        },
        *this);
  }

  T value() const {
    static_assert(std::is_copy_constructible_v<T>,
                  "SynchronizedValue<T>::value() requires a copyable type T");

    return detail::apply([](auto &val) -> T { return val; }, *this);
  }

  template <typename F, typename... ValueTypes>
  friend auto apply(F &&f, synchronized_value_impl<ValueTypes> &...args)
      -> std::invoke_result_t<F, ValueTypes &...>;

  template <typename F, typename... ValueTypes>
  friend auto apply(F &&f, const synchronized_value_impl<ValueTypes> &...args)
      -> std::invoke_result_t<F, ValueTypes &...>;

private:
  T val;
  mutable std::mutex mut{};
};

template <typename F, typename... ValueTypes>
auto apply(F &&f, synchronized_value_impl<ValueTypes> &...args)
    -> std::invoke_result_t<F, ValueTypes &...> {
  static_assert(sizeof...(ValueTypes) > 0, "Cannot call apply() no ValueTypes");

  std::scoped_lock lock{args.mut...};
  return std::invoke(std::forward<F>(f), args.val...);
}

template <typename F, typename... ValueTypes>
auto apply(F &&f, const synchronized_value_impl<ValueTypes> &...args)
    -> std::invoke_result_t<F, ValueTypes &...> {
  static_assert(sizeof...(ValueTypes) > 0, "Cannot call apply() no ValueTypes");

  std::scoped_lock lock{args.mut...};
  return std::invoke(std::forward<F>(f), args.val...);
}

} // namespace detail
