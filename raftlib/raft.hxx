#pragma once

#include "acceptor.hxx"
#include "node_state.hxx"
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
  raft(secret_code, asio::io_context &, const parameters_type &);

private:
  std::shared_ptr<raft> shared_from_this();
  void start_accept();

  template <typename State, typename Behaviour> void process_state();

  asio::io_context &exec_ctx;
  parameters_type parameters;
  acceptor accept;
  utils::synchronized_value<std::variant<follower, candidate, leader>> state;
};

#include "detail/raft.hxx"
