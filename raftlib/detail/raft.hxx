#include <asio/post.hpp>
#include <memory>
#include <spdlog/spdlog.h>

template <typename execution_context>
raft<execution_context>::raft(execution_context &exec_ctx,
                              const raft_options::parameters_type &parameters)
    : exec_ctx{exec_ctx}, parameters{parameters}, acceptor(exec_ctx) {
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
  SPDLOG_INFO("starting execution");
  auto th = shared_from_this();
  acceptor.async_accept([this, th](const asio::error_code &error, auto socket) {
    SPDLOG_DEBUG("got connection");
  });
}

template <typename execution_context> void raft<execution_context>::stop() {
  SPDLOG_DEBUG("stopping execution");
  acceptor.close();
}

template <typename execution_context>
template <typename... Args>
std::weak_ptr<raft<execution_context>>
raft<execution_context>::create(execution_context &exec_ctx, Args &&...args) {
  auto service = std::shared_ptr<raft<execution_context>>(
      new raft{exec_ctx, std::forward<Args>(args)...});
  asio::post(exec_ctx,
             std::bind(&raft<execution_context>::start_accept, service));
  return service;
}
