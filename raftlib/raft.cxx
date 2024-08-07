#include "raft.hxx"
#include "connection.hxx"
#include <asio/read.hpp>
#include <asio/streambuf.hpp>
#include <boost/lexical_cast.hpp>
#include <future>
#include <optional>
#include <utility>
#include <utils/variant.hxx>

raft::raft(secret_code, asio::io_context &exec_ctx,
           const parameters_type &parameters)
    : exec_ctx{exec_ctx}, parameters{parameters}, accept{exec_ctx, parameters},
      state{std::in_place, follower{exec_ctx, parameters.state}} {}

void raft::connect_neighbours() {
  for (const auto &neighbour : parameters.neighbours) {
    connect_neighbour(neighbour);
  }
}

void raft::on_connected(std::shared_ptr<connection_interface<outgoing>> conn) {
  utils::apply(
      [conn](auto &map, auto &timer_map) {
        if (auto endpt = conn->get_endpoint()) {
          auto endptv = *endpt;
          timer_map.erase(endptv);
          map.insert_or_assign(std::move(endptv), conn);
        }
      },
      connection_map, timer_map);
}

void raft::on_disconnected(
    std::shared_ptr<connection_interface<outgoing>> conn) {
  utils::apply(
      [this, conn](auto &connection_map, auto &timer_map) {
        if (auto endpt = conn->get_endpoint()) {
          auto endptv = *endpt;
          connection_map.erase(endptv);
          auto [it, inserted] = timer_map.try_emplace(
              endptv, exec_ctx, parameters.connection.retry);
          auto &[_, timer] = *it;
          auto th = shared_from_this();
          timer.async_wait([this, th, endpt = std::move(endptv)](
                               const asio::error_code &ec) {
            if (!!ec) {
              return;
            }
            connect_neighbour(std::move(endpt));
          });
        }
      },
      connection_map, timer_map);
}

void raft::connect_neighbour(asio::ip::tcp::endpoint endpt) {
  if (utils::apply(
          [&endpt](const auto &map, auto &timer_map) {
            timer_map.erase(endpt);
            return map.find(endpt) != map.cend();
          },
          connection_map, timer_map)) {
    return;
  }

  connection<outgoing>::create(outgoing{std::move(endpt)}, shared_from_this(),
                               parameters.connection);
}

void raft::start_accept() {
  auto th = shared_from_this();
  connection<incoming>::create(incoming{accept}, shared_from_this(),
                               parameters.connection);
}

void raft::stop() { accept.close(); }

void raft::process() {
  utils::apply(
      [this](auto &state) {
        std::visit(overloaded{[this](auto &arg) { process_state(arg); }},
                   state);
      },
      state);
}

template <typename Fn> void raft::change_state(Fn &&fn) {
  utils::apply(std::forward<Fn>(fn), state);
  process();
}

void raft::process_state(follower &state) {
  state.election_timer.expires_after(state.parameters.election_timeout);
  state.election_timer.async_wait([this](const asio::error_code &ec) {
    if (ec) {
      return;
    }
    change_state([](auto &state) {
      state = std::move(candidate(std::get<follower>(state)));
    });
  });
}

void raft::process_state(candidate &state) {
  state.election_timer.expires_after(state.parameters.election_timeout);
  state.election_timer.async_wait([this](const asio::error_code &ec) {
    if (ec) {
      return;
    }
    change_state([](auto &state) {
      state = std::move(leader(std::get<candidate>(state)));
    });
  });
}

void raft::process_state(leader &state) {}
