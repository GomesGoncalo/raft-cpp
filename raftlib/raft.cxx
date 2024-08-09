#include "raft.hxx"
#include "connection.hxx"
#include "node_state.hxx"
#include <asio/read.hpp>
#include <asio/streambuf.hpp>
#include <boost/lexical_cast.hpp>
#include <chrono>
#include <future>
#include <optional>
#include <random>
#include <spdlog/spdlog.h>
#include <utility>
#include <utils/variant.hxx>

namespace {
std::invocable<const asio::error_code &> decltype(auto)
on_success(std::invocable auto &&callback) {
  return [callback = std::forward<decltype(callback)>(callback)](
             const asio::error_code &ec) mutable {
    if (ec) {
      return;
    }

    std::invoke(std::forward<decltype(callback)>(callback));
  };
}
void execute_after(asio::steady_timer &timer,
                   const std::chrono::milliseconds &time,
                   std::invocable<const asio::error_code &> auto &&callback) {
  timer.expires_after(time);
  timer.async_wait(std::forward<decltype(callback)>(callback));
}

template <typename T, typename S>
auto random_time_in_between(const T &min, const S &max) {
  using common_type = std::common_type_t<T, S>;
  const auto common_min = std::chrono::duration_cast<common_type>(min);
  const auto common_max = std::chrono::duration_cast<common_type>(max);
  auto min_count = common_min.count();
  auto max_count = common_max.count();

  if (max_count < min_count) {
    std::swap(min_count, max_count);
  }

  static std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(min_count, max_count);
  return common_type{distribution(generator)};
}
} // namespace

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
