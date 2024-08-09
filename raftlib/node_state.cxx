#include "node_state.hxx"
#include <spdlog/spdlog.h>

follower::follower(asio::io_context &ctx, const state_type &state)
    : state::node{state}, election_timer{ctx} {
  spdlog::info("created follower state");
}

follower::follower(leader &l)
    : state::node{l}, election_timer{l.election_timer.get_executor()} {
  spdlog::info("created follower state from leader");
}

candidate::candidate(follower &f)
    : state::node{f}, election_timer{f.election_timer.get_executor()} {
  spdlog::info("created candidate from follower state");
  start_election();
}

candidate::candidate(candidate &c)
    : state::node{c}, election_timer{c.election_timer.get_executor()} {
  spdlog::info("restart candidate");
  start_election();
}

void candidate::start_election() {
  auto guard = p.acquire_mut();
  guard.currentTerm()++;
  guard.votedFor() = parameters.uuid;
}

leader::leader(candidate &c)
    : state::node(c), election_timer{c.election_timer.get_executor()} {
  spdlog::info("created leader from candidate state");
}
