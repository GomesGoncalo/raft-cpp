#include "raft_options.hxx"
#include <boost/program_options.hpp>
#include <chrono>
#include <filesystem>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <iostream>
#include <spdlog/spdlog.h>
#include <type_traits>
#include <yaml-cpp/yaml.h>

// This must NOT be inside namespaces
template <> struct fmt::formatter<raft_options> : fmt::formatter<string_view> {
  auto format(const raft_options &opt, format_context &ctx) const {
    std::string temp;
    fmt::format_to(
        std::back_inserter(temp),
        "raft_options: {{ threads: {}, timeout: {}ms, address: {}, "
        "neighbours: {} }}",
        opt.threads,
        std::chrono::duration_cast<std::chrono::milliseconds>(opt.timeout)
            .count(),
        opt.address, opt.neighbours);
    return fmt::formatter<string_view>::format(temp, ctx);
  }
};

namespace {
template <typename Key, typename T>
[[nodiscard]] inline
    typename std::enable_if<!std::is_same_v<T, std::chrono::milliseconds>,
                            T>::type
    get(const auto &node, const Key &key) {
  return node[key].template as<T>();
}

template <typename Key, typename T>
[[nodiscard]] inline
    typename std::enable_if<std::is_same_v<T, std::chrono::milliseconds>,
                            T>::type
    get(const auto &node, const Key &key) {
  uint32_t ms = get<Key, uint32_t>(node, key);
  return std::chrono::milliseconds(ms);
}

template <bool Optional, typename Key, typename T>
void get_yaml(const auto &node, T &option, const Key &key) {
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
              const Other &...other) {
  if constexpr (Optional) {
    try {
      get_yaml(node[key], option, other...);
    } catch (const std::exception &ex) {
    }
  } else {
    get_yaml(node[key], option, other...);
  }
}
} // namespace

std::optional<raft_options> raft_options::create(int argc, const char *argv[]) {
  namespace po = boost::program_options;
  std::filesystem::path config;
  raft_options opt;
  po::options_description desc("Allowed options");
  desc.add_options()("help,h", "produce help message")(
      "config,c", po::value(&config), "path to the config")(
      "threads,t",
      po::value(&opt.threads)
          ->default_value(std::thread::hardware_concurrency()),
      "number of threads");

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << desc << "\n";
    return std::nullopt;
  }

  const auto file_config = YAML::LoadFile(config.string());
  get_yaml<false>(file_config, opt.timeout, "timeout");
  get_yaml<true>(file_config, opt.threads, "threads");
  get_yaml<false>(file_config, opt.address, "address");
  get_yaml<false>(file_config, opt.neighbours, "neighbours");

  spdlog::info("Loaded {}", opt);
  return opt;
}
