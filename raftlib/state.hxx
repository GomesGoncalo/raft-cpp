#pragma once

#include "log_entry.hxx"
#include "raft_options.hxx"
#include <boost/uuid/uuid_io.hpp>
#include <optional>
#include <vector>

namespace state {
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
struct const_persistent_guard {
  const_persistent_guard(const persistent &p) : p(p) {}

  const uint32_t &currentTerm() const;
  const std::optional<boost::uuids::uuid> &votedFor() const;
  const std::vector<log_entry> &log() const;

  const persistent &p;
};

struct persistent {
  persistent(const persistent_storage_type &);

  persistent_guard acquire_mut() { return persistent_guard{*this}; }
  const_persistent_guard acquire() const {
    return const_persistent_guard{*this};
  }

  uint32_t get_term() const { return currentTerm; }
  std::optional<boost::uuids::uuid> get_vote() const { return votedFor; }
  std::vector<log_entry> get_log() const { return log; }

private:
  friend class persistent_guard;
  friend class const_persistent_guard;
  uint32_t currentTerm{0};
  std::optional<boost::uuids::uuid> votedFor{std::nullopt};
  std::vector<log_entry> log;
  persistent_storage_type parameters;
};

struct node {
  node(const state_type &);

  persistent p;
  volatiles v{};

  state_type parameters;

protected:
  ~node() = default;
};
} // namespace state
