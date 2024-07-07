// Needs to go first
#include <fmt/ranges.h>

#include "state.hxx"
#include <fmt/format.h>
#include <spdlog/spdlog.h>

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

state::persistent::persistent(const persistent_storage_type &parameters)
    : parameters{parameters} {}

state::node::node(const state_type &config)
    : p{config.persistent_storage}, parameters{config} {}

state::persistent_guard::~persistent_guard() {
  // TODO:write to disk
  SPDLOG_INFO("persistent: {}", p);
}

uint32_t &state::persistent_guard::currentTerm() { return p.currentTerm; }

std::optional<boost::uuids::uuid> &state::persistent_guard::votedFor() {
  return p.votedFor;
}

std::vector<log_entry> &state::persistent_guard::log() { return p.log; }

const uint32_t &state::const_persistent_guard::currentTerm() const {
  return p.currentTerm;
}

const std::optional<boost::uuids::uuid> &
state::const_persistent_guard::votedFor() const {
  return p.votedFor;
}

const std::vector<log_entry> &state::const_persistent_guard::log() const {
  return p.log;
}
