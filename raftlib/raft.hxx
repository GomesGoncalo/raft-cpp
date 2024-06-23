#pragma once

#include "raft_options.hxx"

template <typename execution_context> struct raft final {
  raft(execution_context &exec_ctx, raft_options &&opt);
  ~raft();

  void operator()();

private:
  raft_options opt;
  execution_context &exec_ctx;
};

#include "detail/raft.hxx"
