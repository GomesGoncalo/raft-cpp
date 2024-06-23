#pragma once

#include <chrono>
#include <optional>
#include <vector>

struct raft_options final {
  /**
   * The number of threads to be used by the executor.
   * It defaults to the number of concurrent threads supported by
   * the implementation as returned by `std::thread::hardware_concurrency()` [1]
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
   * The time (in milliseconds) after last hearing a Leader node when a new
   * election should start
   *
   * This value is not optional and must be added in the YAML config file
   *
   * key: timeout
   */
  std::chrono::milliseconds timeout = std::chrono::milliseconds{0};

  /**
   * The address for this node to bind to.
   * It should be formatted as: ip:port
   *
   * This value is not optional and must be added in the YAML config file
   *
   * key: address
   */
  std::string address{};

  /**
   * The addresses where other nodes are reacheable.
   * They should be formatted as: ip:port
   *
   * This value is not optional and must be added in the YAML config file
   *
   * key: neighbours
   */
  std::vector<std::string> neighbours{};

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
