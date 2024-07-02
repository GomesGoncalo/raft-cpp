#pragma once

#include <asio/io_context.hpp>
#include <thread>
#include <vector>

struct concurrency_type;
struct jthread_pool : public asio::io_context {
  void setup_threads(const concurrency_type &concurrency);
  ~jthread_pool();

private:
  std::vector<std::thread> threads;
};
