#include <doctest/doctest.h>

#include <raftlib/raft.hxx>

TEST_SUITE_BEGIN("raft");

TEST_CASE("create raft node") {
  asio::io_context p;
  raft_options opt;
  auto r = raft::create(p, opt.parameters);
}

TEST_SUITE_END();
