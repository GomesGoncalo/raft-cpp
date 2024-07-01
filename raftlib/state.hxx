#pragma once

#include "raft_options.hxx"
#include <boost/uuid/uuid_io.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <optional>

namespace state {
struct log_entry {};

struct volatiles {
  uint64_t commitIndex{0};
  uint64_t lastApplied{0};
};

struct persistent;
struct persistent_guard {
  persistent_guard(persistent &p) : p(p) {}
  ~persistent_guard();

  uint32_t &currentTerm();
  std::optional<boost::uuids::uuid> &votedFor();
  std::vector<log_entry> &log();

  persistent &p;
};

struct persistent {
  persistent(
      const raft_options::parameters_type::state_type::persistent_storage_type
          &) {}

  persistent_guard acquire() { return persistent_guard{*this}; }

  uint32_t get_term() const { return currentTerm; }
  std::optional<boost::uuids::uuid> get_vote() const { return votedFor; }
  std::vector<log_entry> get_log() const { return log; }

private:
  friend class persistent_guard;
  uint32_t currentTerm{0};
  std::optional<boost::uuids::uuid> votedFor{std::nullopt};
  std::vector<log_entry> log;
};

uint32_t &persistent_guard::currentTerm() { return p.currentTerm; }
std::optional<boost::uuids::uuid> &persistent_guard::votedFor() {
  return p.votedFor;
}
std::vector<log_entry> &persistent_guard::log() { return p.log; }

struct node {
  node(const raft_options::parameters_type::state_type &config)
      : p{config.persistent_storage}, parameters{config} {}

  persistent p;
  volatiles v{};

  raft_options::parameters_type::state_type parameters;

protected:
  ~node() = default;
};
} // namespace state
template <>
struct fmt::formatter<state::log_entry> : fmt::formatter<string_view> {
  auto format(const state::log_entry &v, format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp), "");
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <>
struct fmt::formatter<std::optional<boost::uuids::uuid>>
    : fmt::formatter<string_view> {
  auto format(const std::optional<boost::uuids::uuid> &v,
              format_context &ctx) const {
    std::string temp;
    if (v) {
      fmt::format_to(std::back_inserter(temp), "{}",
                     boost::uuids::to_string(*v));
    } else {
      fmt::format_to(std::back_inserter(temp), "none");
    }
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <>
struct fmt::formatter<state::persistent> : fmt::formatter<string_view> {
  auto format(const state::persistent &p, format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp),
                   "{{ term: {}, vote: {}, log: {} }}", p.get_term(),
                   p.get_vote(), p.get_log());
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};

state::persistent_guard::~persistent_guard() {
  // TODO:write to disk
  SPDLOG_INFO("persistent: {}", p);
}
