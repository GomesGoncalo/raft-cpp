#include "jthread_pool.hxx"
#include <ranges>
#include <spdlog/spdlog.h>

void jthread_pool::setup_threads(
    const raft_options::concurrency_type &concurrency) {
  threads.resize(concurrency.threads);
  for (const int i : std::views::iota(1u, concurrency.threads)) {
    SPDLOG_TRACE("Starting executor {}", i);
    threads.emplace_back([this] { run(); });
  }
}

jthread_pool::~jthread_pool() {
  run();
  SPDLOG_TRACE("Joining...");
  for (auto &t : threads) {
    if (t.joinable()) {
      t.join();
      SPDLOG_TRACE("\treleased");
    }
  }
  SPDLOG_TRACE("\t...done");
}
