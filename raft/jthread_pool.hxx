#pragma once

#include <raftlib/raft_options.hxx>

#include <asio/io_service.hpp>
#include <thread>
#include <vector>

struct jthread_pool : public asio::io_service {
  void setup_threads(const raft_options::concurrency_type &concurrency);
  ~jthread_pool();

private:
  std::vector<std::thread> threads;
};
