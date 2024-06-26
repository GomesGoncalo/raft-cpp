#include "connection_interface.hxx"
#include <asio/read.hpp>
#include <asio/streambuf.hpp>
#include <boost/lexical_cast.hpp>
#include <future>
#include <spdlog/spdlog.h>

template <typename execution_context>
raft<execution_context>::raft(secret_code, execution_context &exec_ctx,
                              const raft_options::parameters_type &parameters)
    : exec_ctx{exec_ctx}, parameters{parameters}, acceptor{exec_ctx} {
  acceptor.open(parameters.bind.protocol());
  acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  acceptor.bind(parameters.bind);
  acceptor.listen();
  SPDLOG_INFO("starting raft");
}

template <typename execution_context> raft<execution_context>::~raft() {
  SPDLOG_INFO("terminating raft");
}

template <typename execution_context>
void raft<execution_context>::connect_neighbours() {
  for (const auto &neighbour : parameters.neighbours) {
    connect_neighbour(neighbour);
  }
}

template <typename execution_context>
void raft<execution_context>::on_connected(
    std::shared_ptr<connection_interface<outgoing>> conn) {
  utils::apply(
      [conn](auto &map, auto &timer_map) {
        if (auto endpt = conn->get_endpoint()) {
          auto endptv = *endpt;
          SPDLOG_INFO("Connected");
          timer_map.erase(endptv);
          map.insert_or_assign(std::move(endptv), conn);
        }
      },
      connection_map, timer_map);
}

template <typename execution_context>
void raft<execution_context>::on_disconnected(
    std::shared_ptr<connection_interface<outgoing>> conn) {
  utils::apply(
      [this, conn](auto &connection_map, auto &timer_map) {
        SPDLOG_INFO("Disconnected");
        if (auto endpt = conn->get_endpoint()) {
          auto endptv = *endpt;
          connection_map.erase(endptv);
          auto [it, inserted] = timer_map.try_emplace(
              endptv, exec_ctx, parameters.connection.retry);
          auto &[_, timer] = *it;
          auto th = shared_from_this();
          timer.async_wait([this, th, endpt = std::move(endptv)](
                               const asio::error_code &ec) {
            if (!!ec) {
              SPDLOG_DEBUG("{} timer error: {}", endpt, ec.message);
              return;
            }
            connect_neighbour(std::move(endpt));
          });
        }
      },
      connection_map, timer_map);
}

template <typename execution_context, typename direction>
struct connection : public std::enable_shared_from_this<
                        connection<execution_context, direction>>,
                    public connection_interface<direction> {
private:
  struct secret_code {};

public:
  connection(secret_code, direction &&,
             std::shared_ptr<raft<execution_context>>,
             raft_options::parameters_type::connection_type);

  template <typename Dir, typename... Args>
  static std::weak_ptr<connection<execution_context, direction>>
  create(Dir &&, Args &&...);

  std::optional<asio::ip::tcp::endpoint> get_endpoint() const override;

private:
  auto shared_from_this() {
    return std::enable_shared_from_this<
        connection<execution_context, direction>>::shared_from_this();
  }

  void start(const outgoing &);
  void start(const incoming &);

  raft_options::parameters_type::connection_type parameters;
  std::shared_ptr<raft<execution_context>> node;
  asio::ip::tcp::socket socket;
  direction dir;
};

template <typename execution_context>
void raft<execution_context>::connect_neighbour(asio::ip::tcp::endpoint endpt) {
  if (utils::apply(
          [&endpt](auto &map, auto &timer_map) {
            timer_map.erase(endpt);
            return map.find(endpt) != map.cend();
          },
          connection_map, timer_map)) {
    return;
  }

  connection<execution_context, outgoing>::create(
      outgoing{std::move(endpt)}, shared_from_this(), parameters.connection);
}

template <typename execution_context>
void raft<execution_context>::start_accept() {
  auto th = shared_from_this();
  SPDLOG_INFO("starting accept");
  connection<execution_context, incoming>::create(
      incoming{acceptor}, shared_from_this(), parameters.connection);
}

template <typename execution_context> void raft<execution_context>::stop() {
  SPDLOG_DEBUG("stopping execution");
  acceptor.close();
}

template <typename execution_context>
template <typename... Args>
std::weak_ptr<raft<execution_context>>
raft<execution_context>::create(Args &&...args) {
  auto service = std::make_shared<raft<execution_context>>(
      secret_code{}, std::forward<Args>(args)...);
  service->start_accept();
  service->connect_neighbours();
  return service;
}

#include "connection.hxx"
