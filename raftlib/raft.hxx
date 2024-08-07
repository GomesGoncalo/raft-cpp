#pragma once

#include "acceptor.hxx"
#include "node_state.hxx"
#include <unordered_map>
#include <utils/synchronized_value.hxx>

struct outgoing;
template <typename Dir> struct connection_interface;

struct raft final : public std::enable_shared_from_this<raft> {
private:
  struct secret_code {
    explicit secret_code() = default;
  };

public:
  template <typename... Args> static std::weak_ptr<raft> create(Args &&...);
  void stop();

  raft(secret_code, asio::io_context &, const parameters_type &);

  asio::io_context &get_executor() { return exec_ctx; }

  void on_connected(std::shared_ptr<connection_interface<outgoing>>);
  void on_disconnected(std::shared_ptr<connection_interface<outgoing>>);

private:
  auto shared_from_this() {
    return std::enable_shared_from_this<raft>::shared_from_this();
  }

  void start_accept();
  void connect_neighbours();
  void connect_neighbour(asio::ip::tcp::endpoint);

  asio::io_context &exec_ctx;
  acceptor accept;

  parameters_type parameters;
  utils::synchronized_value<std::unordered_map<
      asio::ip::tcp::endpoint, std::weak_ptr<connection_interface<outgoing>>>>
      connection_map;
  utils::synchronized_value<
      std::unordered_map<asio::ip::tcp::endpoint, asio::steady_timer>>
      timer_map;

  utils::synchronized_value<std::variant<follower, candidate, leader>> state;

  void process();
  template <typename Fn> void change_state(Fn &&);
  void process_state(follower &);
  void process_state(candidate &);
  void process_state(leader &);
};

#include "detail/raft.hxx"
