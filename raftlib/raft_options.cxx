#include "raft_options.hxx"
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <thread>

namespace boost {
template <> spdlog::level::level_enum lexical_cast(const std::string &s) {
  return static_cast<spdlog::level::level_enum>(
      boost::lexical_cast<
          typename std::underlying_type<spdlog::level::level_enum>::type>(s));
}
} // namespace boost

bool raft_options::parse_command_line(int argc, const char *argv[]) {
  namespace po = boost::program_options;
  po::options_description generic("Generic options");
  generic.add_options()("help,h", "Produce help message")(
      "config,c", po::value(&config), "Path to the config yaml file");

  po::options_description concurrency_opt("Threading options");
  concurrency_opt.add_options()(
      "threads,t",
      po::value(&concurrency.threads)
          ->default_value(std::thread::hardware_concurrency()),
      "Number of threads to use")(
      "quit-when-done,q",
      po::value(&concurrency.quit_when_done)
          ->default_value(concurrency.quit_when_done),
      "Quit when there is no more work");

  po::options_description logging_opt("Logging options");
  logging_opt.add_options()(
      "level,l", po::value(&logging.level)->default_value(logging.level),
      "Log level (spdlog enum)")(
      "pattern,p", po::value(&logging.pattern)->default_value(logging.pattern),
      "Log pattern (spdlog types)");

  po::options_description visible;
  visible.add(generic).add(concurrency_opt).add(logging_opt);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, visible), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << visible << "\n";
    return false;
  }

  return true;
}
