#include <asio/post.hpp>
#include <spdlog/spdlog.h>

template <typename execution_context>
raft<execution_context>::raft(execution_context &exec_ctx,
                              const raft_options::parameters_type &opt)
    : exec_ctx{exec_ctx}, opt{opt} {
  spdlog::info("starting raft");
}

template <typename execution_context> raft<execution_context>::~raft() {
  spdlog::info("terminating raft");
}

template <typename execution_context> void raft<execution_context>::init() {
  spdlog::debug("starting execution");
}

template <typename execution_context> void raft<execution_context>::stop() {
  spdlog::debug("stopping execution");
}

template <typename execution_context>
template <typename... Args>
std::weak_ptr<raft<execution_context>>
raft<execution_context>::create(execution_context &exec_ctx, Args &&...args) {
  auto service = std::shared_ptr<raft<execution_context>>(
      new raft{exec_ctx, std::forward<Args>(args)...});
  asio::post(exec_ctx, std::bind(&raft<execution_context>::init, service));
  return service;
}
