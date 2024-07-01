#pragma once

#include "state.hxx"

struct follower : public state::node {
  template <typename execution_context>
  follower(execution_context &ctx,
           const raft_options::parameters_type::state_type &state)
      : election_timer{ctx}, state::node{state} {}

  asio::steady_timer election_timer;
};

struct candidate : public state::node {
  candidate(follower &f)
      : election_timer{f.election_timer.get_executor()}, state::node{f} {
    start_election();
  }
  candidate(candidate &f)
      : election_timer{f.election_timer.get_executor()}, state::node{f} {
    start_election();
  }

  candidate(candidate &&) = default;
  candidate &operator=(candidate &&) = default;

  asio::steady_timer election_timer;

private:
  void start_election() {
    auto guard = p.acquire();
    guard.currentTerm()++;
    guard.votedFor() = parameters.uuid;
  }
};

struct leader : public state::node {
  std::vector<uint64_t> nextIndex{};
  std::vector<uint64_t> matchIndex{};
};
