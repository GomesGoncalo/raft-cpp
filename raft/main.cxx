#include "async_utils.hxx"
#include "formatters.hxx"
#include "jthread_pool.hxx"
#include "logger.hxx"
#include "yaml_conversions.hxx"

#include <raftlib/raft.hxx>
#include <raftlib/raft_options.hxx>

#include <cstdlib>
#include <exception>
#include <spdlog/spdlog.h>
#include <utility>

struct Reader {
  auto operator()(const std::string &path) const {
    return YAML::LoadFile(path);
  }
};

int main(int argc, const char *argv[]) try {
  const auto opt = raft_options::create<Reader>(argc, argv);
  opt.transform([](const raft_options &opt) {
    logger::setup(opt.logging);
    formatters::print(opt);
    jthread_pool p;
    auto w = async_utils::setup_work(p, opt.concurrency);
    auto sig = async_utils::setup_signals(p, std::move(w));
    auto r = raft::create(p, opt.parameters);
    p.setup_threads(opt.concurrency);
    p.run();
    return opt;
  });

  return EXIT_SUCCESS;
} catch (const std::exception &ex) {
  SPDLOG_ERROR("Caught exception: {}", ex.what());
  return EXIT_FAILURE;
} catch (...) {
  SPDLOG_CRITICAL("Caught fatal unknown exception");
  return EXIT_FAILURE;
}
