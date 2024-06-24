#include "jthread_pool.hxx"
#include <ranges>
#include <spdlog/spdlog.h>

void jthread_pool::setup_threads(
    const raft_options::concurrency_type &concurrency) {
  threads.resize(concurrency.threads);
  for (const int i : std::views::iota(1u, concurrency.threads)) {
    SPDLOG_DEBUG("Starting executor {}", i);
    threads.push_back(std::thread([this] { run(); }));
  }
}

jthread_pool::~jthread_pool() {
  run();
  for (auto &t : threads) {
    if (t.joinable())
      t.join();
  }
}
