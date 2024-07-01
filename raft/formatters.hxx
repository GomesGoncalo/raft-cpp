#pragma once

#include <asio/ip/tcp.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <raftlib/raft_options.hxx>
template <>
struct fmt::formatter<raft_options::logging_type>
    : fmt::formatter<string_view> {
  auto format(const raft_options::logging_type &opt,
              format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp), "{{ level: {}, pattern: {} }}",
                   std::to_underlying(opt.level), opt.pattern);
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
struct fmt::formatter<raft_options::parameters_type::connection_type>
    : fmt::formatter<string_view> {
  auto format(const raft_options::parameters_type::connection_type &opt,
              format_context &ctx) const {
    std::string temp;
    fmt::format_to(
        std::back_inserter(temp), "{{ retry: {}ms }}",
        std::chrono::duration_cast<std::chrono::milliseconds>(opt.retry)
            .count());
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <>
struct fmt::formatter<asio::ip::tcp::endpoint> : fmt::formatter<string_view> {
  auto format(const asio::ip::tcp::endpoint &opt, format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp), "{{ {} }}",
                   boost::lexical_cast<std::string>(opt));
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <>
struct fmt::formatter<
    raft_options::parameters_type::state_type::persistent_storage_type>
    : fmt::formatter<string_view> {
  auto format(
      const raft_options::parameters_type::state_type::persistent_storage_type
          &opt,
      format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp), "{{ }}");
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <>
struct fmt::formatter<raft_options::parameters_type::state_type>
    : fmt::formatter<string_view> {
  auto format(const raft_options::parameters_type::state_type &opt,
              format_context &ctx) const {
    std::string temp;
    fmt::format_to(
        std::back_inserter(temp),
        "{{ persistent_storage: {}, election_timeout: {}ms, uuid: {} }}",
        opt.persistent_storage,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            opt.election_timeout)
            .count(),
        boost::uuids::to_string(opt.uuid));
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <>
struct fmt::formatter<raft_options::parameters_type>
    : fmt::formatter<string_view> {
  auto format(const raft_options::parameters_type &opt,
              format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp),
                   "{{ bind: {}, neighbours: {}, connection: "
                   "{}, state: {} }}",
                   opt.bind, opt.neighbours, opt.connection, opt.state);
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
