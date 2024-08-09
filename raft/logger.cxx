#include "logger.hxx"
#include <raftlib/raft_options.hxx>
#include <spdlog/spdlog.h>

void logger::setup(const logging_type &logging) {
  spdlog::set_pattern(logging.pattern);
  spdlog::set_level(logging.level);
  SPDLOG_INFO("Started logger");
}
