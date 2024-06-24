#include <memory>
#include <raftlib/raft.hxx>
#include <raftlib/raft_options.hxx>

#include <asio/executor_work_guard.hpp>
#include <asio/io_service.hpp>
#include <asio/signal_set.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <ranges>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <exception>
#include <utility>

// This must NOT be inside namespaces
template <>
struct fmt::formatter<raft_options::logging_type>
    : fmt::formatter<string_view> {
  auto format(const raft_options::logging_type &opt,
              format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp), "{{ level: {}, pattern {} }}",
                   opt.level, opt.pattern);
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <>
struct fmt::formatter<raft_options::concurrency_type>
    : fmt::formatter<string_view> {
  auto format(const raft_options::concurrency_type &opt,
              format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp),
                   "{{ threads: {}, quit_when_done: {} }}", opt.threads,
                   opt.quit_when_done);
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <>
struct fmt::formatter<raft_options::parameters_type>
    : fmt::formatter<string_view> {
  auto format(const raft_options::parameters_type &opt,
              format_context &ctx) const {
    std::string temp;
    fmt::format_to(
        std::back_inserter(temp),
        "{{ timeout: {}ms, address: {}, neighbours: {} }}",
        std::chrono::duration_cast<std::chrono::milliseconds>(opt.timeout)
            .count(),
        opt.address, opt.neighbours);
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};
template <> struct fmt::formatter<raft_options> : fmt::formatter<string_view> {
  auto format(const raft_options &opt, format_context &ctx) const {
    std::string temp;
    fmt::format_to(std::back_inserter(temp),
                   "{{ concurrency: {}, logging: {}, parameters: {} }}",
                   opt.concurrency, opt.logging, opt.parameters);
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};

struct jthread_pool : public asio::io_service {
  void setup_threads(const raft_options::concurrency_type &concurrency) {
    threads.resize(concurrency.threads);
    for (const int i : std::views::iota(1u, concurrency.threads)) {
      spdlog::info("Starting executor {}", i);
      threads.push_back(std::thread([this] { run(); }));
    }
  }

  ~jthread_pool() {
    run();
    for (auto &t : threads) {
      if (t.joinable())
        t.join();
    }
  }

private:
  std::vector<std::thread> threads;
};

void setup_logger(const raft_options::logging_type &logging) {
  spdlog::set_pattern(logging.pattern);
  spdlog::set_level(static_cast<spdlog::level::level_enum>(logging.level));
  spdlog::info("Started logger");
}

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
      [raft, work = std::move(work)](const asio::error_code &ec,
                                     int signal_number) mutable {
        if (auto l = raft.lock()) {
          spdlog::info("Signal received ({}), cleanup...", signal_number);
          spdlog::trace("raft ownership acquired use_count {}", l.use_count());
          l->stop();
          work = std::nullopt;
          spdlog::info("...done");
        }
      });
  return signals;
}

int main(int argc, const char *argv[]) try {
  const auto opt = raft_options::create(argc, argv);
  opt.transform([](const raft_options &opt) {
    setup_logger(opt.logging);
    spdlog::info("raft_options: {}", opt);
    jthread_pool p;
    auto w = setup_work(p, opt.concurrency);
    auto r = raft<decltype(p)>::create(p, opt.parameters);
    auto sig = setup_signals(p, r, std::move(w));
    p.setup_threads(opt.concurrency);
    p.run();
    return opt;
  });

  return EXIT_SUCCESS;
} catch (const std::exception &ex) {
  spdlog::warn("Caught exception: {}", ex.what());
  return EXIT_FAILURE;
} catch (...) {
  spdlog::error("Caught fatal unknown exception");
  return EXIT_FAILURE;
}
