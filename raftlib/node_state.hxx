#pragma once

#include "state.hxx"

struct leader;
struct follower : public state::node {
  follower(asio::io_context &, const state_type &);
  follower(leader &);

  follower(follower &&) = default;
  follower &operator=(follower &&) = default;
  follower(const follower &) = delete;
  follower &operator=(const follower &) = delete;
  ~follower() = default;

  asio::steady_timer election_timer;
};

struct candidate : public state::node {
  candidate(follower &);
  candidate(candidate &);

  candidate(candidate &&) = default;
  candidate &operator=(candidate &&) = default;
  candidate(const candidate &) = delete;
  candidate &operator=(const candidate &) = delete;
  ~candidate() = default;

  asio::steady_timer election_timer;

private:
  void start_election();
};

struct leader : public state::node {
  leader(candidate &c);

  leader(leader &&) = default;
  leader &operator=(leader &&) = default;
  leader(const leader &) = delete;
  leader &operator=(const leader &) = delete;
  ~leader() = default;

  asio::steady_timer election_timer;
  std::vector<uint64_t> nextIndex{};
  std::vector<uint64_t> matchIndex{};
};
