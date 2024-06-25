#include <asio/post.hpp>
#include <boost/lexical_cast.hpp>
#include <memory>
#include <spdlog/spdlog.h>

template <typename execution_context>
raft<execution_context>::raft(secret_code, execution_context &exec_ctx,
                              const raft_options::parameters_type &parameters)
    : exec_ctx{exec_ctx}, parameters{parameters}, acceptor{exec_ctx} {
  asio::ip::tcp::resolver res(exec_ctx);
  asio::ip::tcp::endpoint endpoint =
      *res.resolve(parameters.address, parameters.port).begin();
  acceptor.open(endpoint.protocol());
  acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  acceptor.bind(endpoint);
  acceptor.listen();
  SPDLOG_INFO("starting raft");
}

template <typename execution_context> raft<execution_context>::~raft() {
  SPDLOG_INFO("terminating raft");
}

template <typename execution_context>
void raft<execution_context>::start_accept() {
  auto th = shared_from_this();
  acceptor.async_accept([this, th](const asio::error_code &error, auto socket) {
    SPDLOG_DEBUG("got connection");
  });
}

template <typename execution_context>
void raft<execution_context>::connect_neighbours() {
  for (const auto &neighbour : parameters.neighbours) {
    const auto delimiter = neighbour.find_last_of(':');
    if (delimiter == std::string::npos) {
      continue;
    }
    std::string ip{neighbour.cbegin(), neighbour.cbegin() + delimiter};
    std::string port{neighbour.cbegin() + delimiter + 1, neighbour.cend()};
    connect_neighbour(std::move(ip), std::move(port));
  }
}

template <typename execution_context>
void raft<execution_context>::connect_neighbour(std::string ip,
                                                std::string port) {
  SPDLOG_INFO("Connecting to {} {}", ip, port);
  asio::ip::tcp::socket socket{exec_ctx};
  asio::ip::tcp::endpoint endpt(asio::ip::address::from_string(ip),
                                boost::lexical_cast<uint16_t>(port));
  // socket.async_connect(std::move(endpt), [] (const asio::error_code& error) {
  //
  // });
  //
  // timer_.expires_from_now(boost::posix_time::seconds(5));
  // timer_.async_wait(boost::bind(&connect_handler::close, this));
}

// struct connection {
//   template <typename execution_context>
//   connection(execution_context &exec_ctx, asio::ip::tcp::endpoint endpt)
//       : socket(exec_ctx), endpt(std::move(endpt)) {}
//
//   asio::ip::tcp::socket socket;
//   asio::ip::tcp::endpoint endpt;
// }

template <typename execution_context> void raft<execution_context>::stop() {
  SPDLOG_DEBUG("stopping execution");
  acceptor.close();
}

template <typename execution_context>
template <typename... Args>
std::weak_ptr<raft<execution_context>>
raft<execution_context>::create(execution_context &exec_ctx, Args &&...args) {
  auto service = std::make_shared<raft<execution_context>>(
      secret_code{}, exec_ctx, std::forward<Args>(args)...);
  asio::post(exec_ctx,
             std::bind(&raft<execution_context>::start_accept, service));
  asio::post(exec_ctx,
             std::bind(&raft<execution_context>::connect_neighbours, service));
  return service;
}
