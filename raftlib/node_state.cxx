#include "node_state.hxx"
#include <spdlog/spdlog.h>

follower::follower(asio::io_context &ctx, const state_type &state)
    : election_timer{ctx}, state::node{state} {
  spdlog::info("created follower state");
}

follower::follower(leader &l)
    : election_timer{l.election_timer.get_executor()}, state::node{l} {
  spdlog::info("created follower state from leader");
}

candidate::candidate(follower &f)
    : election_timer{f.election_timer.get_executor()}, state::node{f} {
  spdlog::info("created candidate from follower state");
  start_election();
}

candidate::candidate(candidate &c)
    : election_timer{c.election_timer.get_executor()}, state::node{c} {
  spdlog::info("restart candidate");
  start_election();
}

void candidate::start_election() {
  auto guard = p.acquire_mut();
  guard.currentTerm()++;
  guard.votedFor() = parameters.uuid;
}

leader::leader(candidate &c)
    : election_timer{c.election_timer.get_executor()}, state::node(c) {
  spdlog::info("created leader from candidate state");
}
