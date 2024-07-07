#pragma once

#include <type_traits>
#include <utils/constexpr.hxx>
#include <utils/variant.hxx>

// The order in this enum will define their value
using RPCType = std::variant<AppendEntries, RequestVote, AppendEntriesResponse,
                             RequestVoteResponse>;

template <typename Nested, typename Type>
constexpr unsigned int find_in_nested() {
  if constexpr (is_variant<Nested>::value) {
    unsigned int sum = 0;
    constexpr_for<0, std::variant_size_v<Nested>, 1>([&sum](auto i) {
      sum += find_in_nested<std::variant_alternative_t<i, Nested>, Type>();
    });
    return sum;
  } else {
    return std::is_same_v<Nested, Type> ? 1 : 0;
  }
}

template <typename Type> constexpr unsigned int get_num_types() {
  if constexpr (is_variant<Type>::value) {
    unsigned int sum = 0;
    constexpr_for<0, std::variant_size_v<Type>, 1>([&sum](auto i) {
      sum += get_num_types<std::variant_alternative_t<i, Type>>();
    });
    return sum;
  } else {
    return 1;
  }
}

template <typename Nested, typename Sparse> constexpr bool validate_types() {
  static_assert(get_num_types<Sparse>() == get_num_types<Nested>(),
                "number of types should be the same");

  constexpr_for<0, std::variant_size_v<Sparse>, 1>([](auto i) {
    static_assert(!is_variant<std::variant_alternative_t<i, Sparse>>::value,
                  "cannot have variant in the sparse type");
    static_assert(
        find_in_nested<Nested, std::variant_alternative_t<i, Sparse>>() == 1,
        "MUST have one and only one corresponding type");
  });
  return true;
}

static_assert(validate_types<MessageType, RPCType>());
