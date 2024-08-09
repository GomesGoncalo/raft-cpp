#pragma once

#include <concepts>
#include <random>
#include <type_traits>

namespace detail {
template <typename T> struct is_chrono_duration : std::false_type {};

template <typename Rep, typename Period>
struct is_chrono_duration<std::chrono::duration<Rep, Period>> : std::true_type {
};

template <typename T>
inline constexpr bool is_chrono_duration_v = is_chrono_duration<T>::value;
} // namespace detail

template <typename T>
concept ChronoDuration = detail::is_chrono_duration_v<T>;

template <typename T, typename S>
auto random_time_in_between(const T &min, const S &max) {
  using common_type = std::common_type_t<T, S>;
  const auto common_min = std::chrono::duration_cast<common_type>(min);
  const auto common_max = std::chrono::duration_cast<common_type>(max);
  auto min_count = common_min.count();
  auto max_count = common_max.count();

  if (max_count < min_count) {
    std::swap(min_count, max_count);
  }

  static std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(min_count, max_count);
  return common_type{distribution(generator)};
}
