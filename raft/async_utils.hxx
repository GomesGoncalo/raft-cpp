#include <raftlib/raft.hxx>
#include <raftlib/raft_options.hxx>

#include <asio/executor_work_guard.hpp>
#include <asio/io_service.hpp>
#include <asio/signal_set.hpp>

namespace async_utils {
std::optional<asio::executor_work_guard<asio::io_service::executor_type>>
setup_work(asio::io_service &io_ctx,
           const raft_options::concurrency_type &concurrency) {
  if (concurrency.quit_when_done) {
    return {};
  } else {
    return asio::executor_work_guard<asio::io_service::executor_type>(
        io_ctx.get_executor());
  }
}

template <typename T>
auto setup_signals(
    auto &io_ctx, std::weak_ptr<raft<T>> raft,
    std::optional<asio::executor_work_guard<asio::io_service::executor_type>>
        &&work) {
  auto signals = std::make_unique<asio::signal_set>(io_ctx, SIGINT, SIGTERM);
  signals->async_wait(
      [raft, work = std::move(work), &io_ctx](const asio::error_code &ec,
                                              int signal_number) mutable {
        if (auto l = raft.lock()) {
          SPDLOG_INFO("Signal received ({}), cleanup...", signal_number);
          SPDLOG_TRACE("raft ownership acquired use_count {}", l.use_count());
          work = std::nullopt;
          io_ctx.stop();
          SPDLOG_INFO("...done");
        }
      });
  return signals;
}
} // namespace async_utils
