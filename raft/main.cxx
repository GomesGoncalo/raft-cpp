
#include "async_utils.hxx"
#include "formatters.hxx"
#include "jthread_pool.hxx"
#include "logger.hxx"

#include <raftlib/raft.hxx>
#include <raftlib/raft_options.hxx>

#include <cstdlib>
#include <exception>
#include <spdlog/spdlog.h>
#include <utility>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <yaml-cpp/yaml.h>

namespace YAML {
template <> struct convert<boost::uuids::uuid> {
  static bool decode(const Node &node, boost::uuids::uuid &out) {
    std::string s;
    if (!convert<decltype(s)>::decode(node, s)) {
      return false;
    }

    boost::uuids::string_generator gen;
    out = gen(s);
    return true;
  }
};
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
    auto r = raft::create(p, opt.parameters);
    auto sig = async_utils::setup_signals(p, r, std::move(w));
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
