#include <raftlib/raft.hxx>
#include <raftlib/raft_options.hxx>

#include <asio/thread_pool.hpp>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <exception>

int main(int argc, const char *argv[]) try {
  auto opt = raft_options::create(argc, argv);
  if (!opt)
    return EXIT_SUCCESS;

  auto opt_val = std::move(*opt);
  asio::thread_pool p{opt_val.threads};
  spdlog::info("Started {} threads", opt_val.threads);
  raft r{p, std::move(opt_val)};
  r();
  return EXIT_SUCCESS;
} catch (const std::exception &ex) {
  spdlog::warn("Caught exception: {}", ex.what());
  return EXIT_FAILURE;
} catch (...) {
  spdlog::error("Caught fatal unknown exception");
  return EXIT_FAILURE;
}
