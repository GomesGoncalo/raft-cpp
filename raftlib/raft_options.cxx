#include "raft_options.hxx"
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <thread>
#include <type_traits>
#include <utility>
#include <yaml-cpp/yaml.h>

namespace boost {
template <> spdlog::level::level_enum lexical_cast(const std::string &s) {
  return static_cast<spdlog::level::level_enum>(
      boost::lexical_cast<
          typename std::underlying_type<spdlog::level::level_enum>::type>(s));
}
} // namespace boost

namespace YAML {
template <> struct convert<asio::ip::tcp::endpoint> {
  static bool decode(const Node &node, asio::ip::tcp::endpoint &out) {
    std::string s;
    if (!convert<decltype(s)>::decode(node, s)) {
      return false;
    }

    const auto delimiter = s.find_last_of(':');
    if (delimiter == std::string::npos) {
      throw std::runtime_error(fmt::format("missing port in address: {}", s));
      return false;
    }
    const std::string ip{s.cbegin(), s.cbegin() + delimiter};
    const std::string port{s.cbegin() + delimiter + 1, s.cend()};

    try {
      out = asio::ip::tcp::endpoint{asio::ip::address::from_string(ip),
                                    boost::lexical_cast<uint16_t>(port)};
    } catch (const std::exception &e) {
      throw std::runtime_error(
          fmt::format("error parsing {} error: {}", s, e.what()));
    }
    return true;
  }
};
template <> struct convert<std::unordered_set<asio::ip::tcp::endpoint>> {
  static bool decode(const Node &node,
                     std::unordered_set<asio::ip::tcp::endpoint> &out) {
    std::vector<asio::ip::tcp::endpoint> v;
    if (!convert<decltype(v)>::decode(node, v)) {
      return false;
    }

    std::copy(v.cbegin(), v.cend(), std::inserter(out, out.end()));
    return true;
  }
};
template <> struct convert<spdlog::level::level_enum> {
  static bool decode(const Node &node, spdlog::level::level_enum &out) {
    std::underlying_type<spdlog::level::level_enum>::type proxy;
    if (!convert<decltype(proxy)>::decode(node, proxy))
      return false;

    out = static_cast<spdlog::level::level_enum>(proxy);
    return true;
  }
};
} // namespace YAML

namespace {
template <typename T> struct is_chrono_duration : std::false_type {};

template <typename Rep, typename Period>
struct is_chrono_duration<std::chrono::duration<Rep, Period>> : std::true_type {
};

template <typename Key, typename T>
[[nodiscard]] inline
    typename std::enable_if<!is_chrono_duration<T>::value, T>::type
    get(const auto &node, const Key &key) {
  return node[key].template as<T>();
}

template <typename Key, typename T>
[[nodiscard]] inline
    typename std::enable_if<is_chrono_duration<T>::value, T>::type
    get(const auto &node, const Key &key) {
  typename T::rep duration = get<Key, typename T::rep>(node, key);
  return T{duration};
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
  get_yaml<false>(file_config, opt.parameters.bind, "parameters", "bind");
  get_yaml<false>(file_config, opt.parameters.neighbours, "parameters",
                  "neighbours");
  get_yaml<false>(file_config, opt.parameters.connection.retry, "parameters",
                  "connection", "retry");
  return opt;
}
