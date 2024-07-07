#pragma once

#include <variant>

template <class... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <class... Args> struct variant_cast_proxy {
  std::variant<Args...> v;

  template <class... ToArgs> operator std::variant<ToArgs...>() {
    return std::visit([](auto &&arg) -> std::variant<ToArgs...> { return arg; },
                      v);
  }
};

template <typename VariantType, typename T, std::size_t index = 0>
constexpr bool variant_contains() {
  if constexpr (index == std::variant_size_v<VariantType>) {
    return false;
  } else if constexpr (std::is_same_v<
                           std::variant_alternative_t<index, VariantType>, T>) {
    return true;
  } else {
    return variant_contains<VariantType, T, index + 1>();
  }
}

template <typename T> struct is_variant : std::false_type {};

template <typename... Args>
struct is_variant<std::variant<Args...>> : std::true_type {};

template <class... Args>
auto variant_cast(std::variant<Args...> &&v) -> variant_cast_proxy<Args...> {
  return {std::move(v)};
}
