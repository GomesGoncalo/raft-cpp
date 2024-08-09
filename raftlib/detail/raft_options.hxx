#pragma once

#include <utils/chrono.hxx>

namespace detail {
template <typename Key, typename T>
  requires(!ChronoDuration<T>)
[[nodiscard]] inline T get(const auto &node, const Key &key) {
  return node[key].template as<T>();
}

template <typename Key, typename T>
  requires(ChronoDuration<T>)
[[nodiscard]] inline T get(const auto &node, const Key &key) {
  typename T::rep duration = get<Key, typename T::rep>(node, key);
  return T{duration};
}

template <bool Optional, typename Key, typename T>
void get_yaml(const auto &node, T &option, const Key &key) noexcept(Optional) {
  if constexpr (Optional) {
    try {
      option = std::forward<T>(get<Key, T>(node, key));
    } catch (const std::exception &ex) {
    }
  } else {
    option = std::forward<T>(get<Key, T>(node, key));
  }
}
template <bool Optional, typename Key, typename T, typename... Other>
void get_yaml(const auto &node, T &option, const Key &key,
              const Other &...other) noexcept(Optional) {
  if constexpr (Optional) {
    try {
      get_yaml<Optional>(node[key], option, other...);
    } catch (const std::exception &ex) {
    }
  } else {
    get_yaml<Optional>(node[key], option, other...);
  }
}
} // namespace detail

template <typename Reader>
  requires std::invocable<const Reader &, const std::string &>
std::optional<raft_options> raft_options::create(int argc, const char *argv[],
                                                 Reader &&reader) {
  raft_options opt;
  if (!opt.parse_command_line(argc, argv)) {
    return std::nullopt;
  }

  const auto file_config = reader(opt.config.string());
  detail::get_yaml<true>(file_config, opt.concurrency.threads, "concurrency",
                         "threads");
  detail::get_yaml<true>(file_config, opt.concurrency.quit_when_done,
                         "concurrency", "quit-when-done");
  detail::get_yaml<true>(file_config, opt.logging.level, "logging", "level");
  detail::get_yaml<true>(file_config, opt.logging.pattern, "logging",
                         "pattern");
  detail::get_yaml<false>(file_config, opt.parameters.bind, "parameters",
                          "bind");
  detail::get_yaml<false>(file_config, opt.parameters.neighbours, "parameters",
                          "neighbours");
  detail::get_yaml<false>(file_config, opt.parameters.connection.retry,
                          "parameters", "connection", "retry");
  detail::get_yaml<false>(file_config, opt.parameters.state.uuid, "parameters",
                          "state", "uuid");
  detail::get_yaml<false>(file_config, opt.parameters.state.election_start_min,
                          "parameters", "state", "election-start-min");
  detail::get_yaml<false>(file_config, opt.parameters.state.election_start_max,
                          "parameters", "state", "election-start-max");
  detail::get_yaml<false>(file_config, opt.parameters.state.election_timeout,
                          "parameters", "state", "election-timeout");
  return opt;
}
