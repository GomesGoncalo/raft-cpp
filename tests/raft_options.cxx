#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <raftlib/raft_options.hxx>

TEST_SUITE_BEGIN("raft options");

TEST_CASE(
    "raft options returns empty optional when not valid options are passed") {
  std::array<const char *, 2> data = {"raft", "--invalid_param"};
  try {
    auto opt = raft_options::create(data.size(), data.data());
    REQUIRE(false);
  } catch (const std::exception &ex) {
    REQUIRE(true);
  }
}

TEST_CASE("raft options returns empty optional when help is passed") {
  std::array<const char *, 2> data = {"raft", "--help"};
  auto opt = raft_options::create(data.size(), data.data());
  REQUIRE(!opt.has_value());
}

TEST_SUITE_END();
