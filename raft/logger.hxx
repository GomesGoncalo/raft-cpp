#pragma once

namespace logger {
void setup(const raft_options::logging_type &logging) {
  spdlog::set_pattern(logging.pattern);
  spdlog::set_level(static_cast<spdlog::level::level_enum>(logging.level));
  SPDLOG_INFO("Started logger");
}
} // namespace logger
