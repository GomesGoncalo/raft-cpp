#pragma once

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <raftlib/raft_options.hxx>

template <>
struct fmt::formatter<raft_options::logging_type>
    : fmt::formatter<string_view> {
  auto format(const raft_options::logging_type &opt,
              format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp), "{{ level: {}, pattern {} }}",
                   opt.level, opt.pattern);
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <>
struct fmt::formatter<raft_options::concurrency_type>
    : fmt::formatter<string_view> {
  auto format(const raft_options::concurrency_type &opt,
              format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp),
                   "{{ threads: {}, quit_when_done: {} }}", opt.threads,
                   opt.quit_when_done);
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <>
struct fmt::formatter<raft_options::parameters_type>
    : fmt::formatter<string_view> {
  auto format(const raft_options::parameters_type &opt,
              format_context &ctx) const {
    std::string temp;
    fmt::format_to(
        std::back_inserter(temp),
        "{{ timeout: {}ms, address: {}, port: {}, neighbours: {} }}",
        std::chrono::duration_cast<std::chrono::milliseconds>(opt.timeout)
            .count(),
        opt.address, opt.port, opt.neighbours);
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <> struct fmt::formatter<raft_options> : fmt::formatter<string_view> {
  auto format(const raft_options &opt, format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp),
                   "{{ concurrency: {}, logging: {}, parameters: {} }}",
                   opt.concurrency, opt.logging, opt.parameters);
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
