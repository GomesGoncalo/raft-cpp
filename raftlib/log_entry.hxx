#pragma once

#include <fmt/format.h>

struct log_entry {};

template <> struct fmt::formatter<log_entry> : fmt::formatter<string_view> {
  auto format(const log_entry &v, format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp), "");
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
