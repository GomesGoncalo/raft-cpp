#pragma once

#include "log_entry.hxx"
#include <asio/streambuf.hpp>
#include <boost/uuid/uuid.hpp>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

struct AppendEntries {
  uint32_t term;
  boost::uuids::uuid leaderId;
  uint32_t prevLogIndex;
  std::vector<log_entry> entries;
  uint32_t leaderCommit;
};

struct AppendEntriesResponse {
  uint32_t term;
  bool success;
};

struct RequestVote {
  uint32_t term;
  boost::uuids::uuid candidateId;
  uint32_t lastLogIndex;
  uint32_t lastLogTerm;
};

struct RequestVoteResponse {
  uint32_t term;
  bool voteGranted;
};

using RequestType = std::variant<AppendEntries, RequestVote>;
using ResponseType = std::variant<AppendEntriesResponse, RequestVoteResponse>;
using MessageType = std::variant<RequestType, ResponseType>;

void serialize(RequestType);
void serialize(ResponseType);

std::optional<MessageType> deserialize();
