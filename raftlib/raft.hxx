#pragma once

#include "raft_options.hxx"
#include <asio/ip/tcp.hpp>
#include <boost/uuid/uuid.hpp>
#include <unordered_map>

template <typename execution_context>
struct raft final
    : public std::enable_shared_from_this<raft<execution_context>> {
private:
  struct secret_code {};

public:
  template <typename... Args>
  static std::weak_ptr<raft<execution_context>> create(execution_context &,
                                                       Args &&...);
  ~raft();
  void stop();

  raft(secret_code, execution_context &, const raft_options::parameters_type &);

private:
  auto shared_from_this() {
    return std::enable_shared_from_this<
        raft<execution_context>>::shared_from_this();
  }

  void start_accept();
  void connect_neighbours();
  void connect_neighbour(std::string ip, std::string port);

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

  raft_options::parameters_type parameters;
  execution_context &exec_ctx;
  asio::ip::tcp::acceptor acceptor;

  // struct connection;
  // std::unordered_map<asio::ip::tcp::endpoint, std::weak_ptr<connection>>;
};

#include "detail/raft.hxx"
