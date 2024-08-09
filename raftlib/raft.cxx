#include "raft.hxx"
#include "connection.hxx"
#include "node_state.hxx"
#include <utils/on_success.hxx>
#include <utils/timer.hxx>
#include <utils/variant.hxx>

#include <spdlog/spdlog.h>

raft::raft(secret_code, asio::io_context &exec_ctx,
           const parameters_type &parameters)
    : exec_ctx{exec_ctx}, parameters{parameters}, accept{exec_ctx, parameters},
      state{follower{exec_ctx, parameters.state}} {}

void raft::start_accept() {
  auto th = shared_from_this();
  connection<incoming>::create(incoming{accept}, exec_ctx, shared_from_this(),
                               parameters.connection);
}

std::shared_ptr<raft> raft::shared_from_this() {
  return std::enable_shared_from_this<raft>::shared_from_this();
}

struct FromStateChangeBehaviour {};
struct StartElection {};
struct MoveToNext {};

template <> void raft::process_state<candidate, FromStateChangeBehaviour>();

template <> void raft::process_state<follower, MoveToNext>() {
  utils::apply(
      [](auto &state) {
        spdlog::info("follower moving to candidate");
        state = std::move(candidate(std::get<follower>(state)));
      },
      state);
  process_state<candidate, FromStateChangeBehaviour>();
}

template <> void raft::process_state<leader, FromStateChangeBehaviour>();
template <> void raft::process_state<candidate, MoveToNext>() {
  utils::apply(
      [](auto &state) {
        state = std::move(leader(std::get<candidate>(state)));
      },
      state);
  process_state<leader, FromStateChangeBehaviour>();
}

template <> void raft::process_state<candidate, StartElection>() {
  utils::apply(
      [this](auto &inner) {
        spdlog::info("starting election");
        candidate new_state(std::get<candidate>(inner));
        // TODO: Send RequestVoteRPC and wait for responses
        execute_after(new_state.election_timer,
                      new_state.parameters.election_timeout, on_success([this] {
                        process_state<candidate, FromStateChangeBehaviour>();
                      }));
        inner = std::move(new_state);
      },
      state);
}

template <> void raft::process_state<follower, FromStateChangeBehaviour>();
template <> void raft::process_state<follower, EntryPoint>() {
  spdlog::info("Starting the state machine");
  process_state<follower, FromStateChangeBehaviour>();
}

template <> void raft::process_state<follower, FromStateChangeBehaviour>() {
  utils::apply(
      [this](auto &inner) {
        follower &f = std::get<follower>(inner);
        spdlog::info("scheduling move to candidate");
        execute_after(
            f.election_timer, f.parameters.election_timeout,
            on_success([this] { process_state<follower, MoveToNext>(); }));
      },
      state);
}

template <> void raft::process_state<candidate, FromStateChangeBehaviour>() {
  utils::apply(
      [this](auto &inner) {
        candidate &f = std::get<candidate>(inner);
        const auto next_election_in = random_time_in_between(
            f.parameters.election_start_min, f.parameters.election_start_max);
        spdlog::info("Schedule next election in {} ms",
                     std::chrono::duration_cast<std::chrono::milliseconds>(
                         next_election_in)
                         .count());
        execute_after(f.election_timer, next_election_in, on_success([this] {
                        process_state<candidate, StartElection>();
                      }));
      },
      state);
}

template <> void raft::process_state<leader, FromStateChangeBehaviour>() {}
