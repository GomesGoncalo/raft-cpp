#pragma once

#include "raft_options.hxx"
#include <asio/ip/tcp.hpp>
#include <boost/uuid/uuid.hpp>

template <typename execution_context>
struct raft final
    : public std::enable_shared_from_this<raft<execution_context>> {
  template <typename... Args>
  static std::weak_ptr<raft<execution_context>> create(execution_context &,
                                                       Args &&...);
  ~raft();
  void stop();

private:
  auto shared_from_this() {
    return std::enable_shared_from_this<
        raft<execution_context>>::shared_from_this();
  }

  raft(execution_context &, const raft_options::parameters_type &);
  void start_accept();

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
};

#include "detail/raft.hxx"
