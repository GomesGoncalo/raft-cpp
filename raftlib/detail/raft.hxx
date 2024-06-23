#include <spdlog/spdlog.h>

template <typename execution_context>
raft<execution_context>::raft(execution_context &exec_ctx, raft_options &&opt)
    : exec_ctx{exec_ctx}, opt{std::move(opt)} {
  spdlog::set_pattern("[%D %H:%M:%S.%f %z] [%^%l%$] [thread %t] %v");
  spdlog::info("starting raft");
}

template <typename execution_context> raft<execution_context>::~raft() {
  spdlog::info("terminating raft");
}

template <typename execution_context>
void raft<execution_context>::operator()() {}
