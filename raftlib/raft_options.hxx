#pragma once

#include <asio/ip/tcp.hpp>
#include <boost/uuid/uuid.hpp>
#include <chrono>
#include <filesystem>
#include <optional>
#include <spdlog/spdlog.h>
#include <unordered_set>
#include <vector>

using namespace std::chrono_literals;
struct raft_options final {
  /**
   * The path to the config file.
   * It should contain a YAML document.
   */
  std::filesystem::path config;

  struct concurrency_type {
    /**
     * The number of threads to be used by the executor.
     * It defaults to the number of concurrent threads supported by
     * the implementation as returned by `std::thread::hardware_concurrency()`
     * [1]
     *
     * This value can be provided in the YAML config file (preferred) or in the
     * command line. But it is optional.
     *
     * key: threads
     *
     * [1]: https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency
     */
    uint32_t threads{0};

    /**
     * If true the program will terminate when there is no more work to do; this
     * behaviour should be the default. If you want the program to keep running
     * set this to false.
     */
    bool quit_when_done{true};
  } concurrency{};

  struct logging_type {
    /**
     * The spdlog log level.
     * ```
     * #define SPDLOG_LEVEL_TRACE 0
     * #define SPDLOG_LEVEL_DEBUG 1
     * #define SPDLOG_LEVEL_INFO 2
     * #define SPDLOG_LEVEL_WARN 3
     * #define SPDLOG_LEVEL_ERROR 4
     * #define SPDLOG_LEVEL_CRITICAL 5
     * #define SPDLOG_LEVEL_OFF 6
     * ```
     * This value can be provided in the YAML config file (preferred) or in the
     * command line. But it is optional.
     *
     * key: logging.level
     */
    spdlog::level::level_enum level{spdlog::level::info};

    /**
     * The spdlog log pattern.
     *
     * This value can be provided in the YAML config file (preferred) or in the
     * command line. But it is optional.
     *
     * key: logging.pattern
     */
    std::string pattern{
        "[%D %H:%M:%S.%f %z] [%^%l%$] [thread %t] [%s:%!:%#] %v"};
  } logging{};

  struct parameters_type {
    /**
     * The address for this node to bind to.
     *
     * This value is not optional and must be added in the YAML config file
     *
     * key: address
     */
    asio::ip::tcp::endpoint bind{};

    /**
     * The addresses where other nodes are reacheable.
     * They should be formatted as: ip:port
     *
     * This value is not optional and must be added in the YAML config file
     *
     * key: neighbours
     */
    std::unordered_set<asio::ip::tcp::endpoint> neighbours{};

    struct connection_type {
      /**
       * The time before retrying connecting to a neighbour in milliseconds
       *
       * This value is not optional and must be added in the YAML config file
       *
       * key: retry
       */
      std::chrono::milliseconds retry{0ms};
    } connection{};

    struct state_type {
      struct persistent_storage_type {
      } persistent_storage{};

      /**
       * The time (in milliseconds) after last hearing a Leader node when a new
       * election should start
       *
       * This value is not optional and must be added in the YAML config file
       *
       * key: election-timeout
       */
      std::chrono::milliseconds election_timeout{0ms};
      boost::uuids::uuid uuid{};
    } state{};
  } parameters{};

  /**
   * Creates this structure from the command line arguments.
   *
   * Will return empty optional for any input that makes the program terminate
   * but is not an error (e.g. --help)
   *
   * A config file will be required due to the mandatory parameters above that
   * are only accepted there.
   *
   * Will throw for any unknown or malformated parameters (both in the command
   * line or in the YAML config file)
   */
  [[nodiscard]] static std::optional<raft_options> create(int argc,
                                                          const char *argv[]);
};
