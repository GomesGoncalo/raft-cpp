#include "raft_options.hxx"
#include <boost/program_options.hpp>
#include <iostream>
#include <thread>
#include <yaml-cpp/yaml.h>

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
      get_yaml<Optional>(node[key], option, other...);
    } catch (const std::exception &ex) {
    }
  } else {
    get_yaml<Optional>(node[key], option, other...);
  }
}
} // namespace

std::optional<raft_options> raft_options::create(int argc, const char *argv[]) {
  namespace po = boost::program_options;
  raft_options opt;
  po::options_description generic("Generic options");
  generic.add_options()("help,h", "Produce help message")(
      "config,c", po::value(&opt.config), "Path to the config yaml file");

  po::options_description concurrency("Threading options");
  concurrency.add_options()(
      "threads,t",
      po::value(&opt.concurrency.threads)
          ->default_value(std::thread::hardware_concurrency()),
      "Number of threads to use")(
      "quit-when-done,q",
      po::value(&opt.concurrency.quit_when_done)
          ->default_value(opt.concurrency.quit_when_done),
      "Quit when there is no more work");

  po::options_description logging("Logging options");
  logging.add_options()(
      "level,l",
      po::value(&opt.logging.level)->default_value(opt.logging.level),
      "Log level (spdlog enum)")(
      "pattern,p",
      po::value(&opt.logging.pattern)->default_value(opt.logging.pattern),
      "Log pattern (spdlog types)");

  po::options_description visible;
  visible.add(generic).add(concurrency).add(logging);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, visible), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << visible << "\n";
    return std::nullopt;
  }

  const auto file_config = YAML::LoadFile(opt.config.string());
  get_yaml<true>(file_config, opt.concurrency.threads, "concurrency",
                 "threads");
  get_yaml<true>(file_config, opt.concurrency.quit_when_done, "concurrency",
                 "quit-when-done");
  get_yaml<true>(file_config, opt.logging.level, "logging", "level");
  get_yaml<true>(file_config, opt.logging.pattern, "logging", "pattern");
  get_yaml<false>(file_config, opt.parameters.timeout, "parameters", "timeout");
  get_yaml<false>(file_config, opt.parameters.address, "parameters", "address");
  get_yaml<false>(file_config, opt.parameters.neighbours, "parameters",
                  "neighbours");
  return opt;
}
