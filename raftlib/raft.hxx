#pragma once

#include "detail/connection_interface.hxx"
#include "raft_options.hxx"
#include "synchronized_value.hxx"
#include <boost/uuid/uuid.hpp>
#include <unordered_map>

template <typename execution_context>
struct raft final
    : public std::enable_shared_from_this<raft<execution_context>> {
private:
  struct secret_code {};

public:
  template <typename... Args>
  static std::weak_ptr<raft<execution_context>> create(Args &&...);
  ~raft();
  void stop();

  raft(secret_code, execution_context &, const raft_options::parameters_type &);

  execution_context &get_executor() { return exec_ctx; }

  void on_connected(std::shared_ptr<connection_interface<outgoing>>);
  void on_disconnected(std::shared_ptr<connection_interface<outgoing>>);

private:
  auto shared_from_this() {
    return std::enable_shared_from_this<
        raft<execution_context>>::shared_from_this();
  }

  void start_accept();
  void connect_neighbours();
  void connect_neighbour(asio::ip::tcp::endpoint);

  struct log_entry {};

  struct volatile_state {
    uint64_t commitIndex{0};
    uint64_t lastApplied{0};
  };

  struct persistent_state {
  private:
    uint32_t currentTerm{0};
    std::optional<boost::uuids::uuid> votedFor{std::nullopt};
    std::vector<log_entry> log;
  };

  execution_context &exec_ctx;
  asio::ip::tcp::acceptor acceptor;

  raft_options::parameters_type parameters;
  utils::synchronized_value<std::unordered_map<
      asio::ip::tcp::endpoint, std::weak_ptr<connection_interface<outgoing>>>>
      connection_map;
  utils::synchronized_value<
      std::unordered_map<asio::ip::tcp::endpoint, asio::steady_timer>>
      timer_map;
};

#include "detail/raft.hxx"
