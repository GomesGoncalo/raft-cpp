#include "async_utils.hxx"
#include <raftlib/raft.hxx>
#include <raftlib/raft_options.hxx>

std::optional<asio::executor_work_guard<asio::io_context::executor_type>>
async_utils::setup_work(asio::io_context &io_ctx,
                        const concurrency_type &concurrency) {
  if (concurrency.quit_when_done) {
    return {};
  } else {
    return asio::executor_work_guard<asio::io_context::executor_type>(
        io_ctx.get_executor());
  }
}

std::unique_ptr<asio::signal_set> async_utils::setup_signals(
    asio::io_context &io_ctx,
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>>
        &&work) {
  auto signals = std::make_unique<asio::signal_set>(io_ctx, SIGINT, SIGTERM);
  signals->async_wait(
      [work = std::move(work), &io_ctx](const asio::error_code &ec,
                                        int signal_number) mutable {
        work = std::nullopt;
        io_ctx.stop();
      });
  return signals;
}
