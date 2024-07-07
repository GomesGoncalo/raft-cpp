#include "message.hxx"
#include "detail/message_sparse.hxx"
#include <arpa/inet.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

struct SerializationProxy {
  uint8_t tag;
  RPCType rest;
};

namespace boost {
namespace serialization {
template <typename Archive>
void save(Archive &ar, const boost::uuids::uuid &uuid,
          const unsigned int version) {
  ar &boost::serialization::make_array(uuid.data, uuid.size());
}
template <typename Archive>
void save(Archive &ar, const log_entry &entry, const unsigned int version) {}
template <typename Archive>
void save(Archive &ar, const RequestVote &reqVote, const unsigned int version) {
  ar &htonl(reqVote.term);
  ar & reqVote.candidateId;
  ar &htonl(reqVote.lastLogIndex);
  ar &htonl(reqVote.lastLogTerm);
}
template <typename Archive>
void save(Archive &ar, const RequestVoteResponse &reqVoteRes,
          const unsigned int version) {
  ar &htonl(reqVoteRes.term);
  ar & reqVoteRes.voteGranted;
}
template <typename Archive>
void save(Archive &ar, const AppendEntries &appendEntries,
          const unsigned int version) {
  ar &htonl(appendEntries.term);
  ar & appendEntries.leaderId;
  ar &htonl(appendEntries.prevLogIndex);
  ar & appendEntries.entries;
  ar &htonl(appendEntries.leaderCommit);
}
template <typename Archive>
void save(Archive &ar, const AppendEntriesResponse &appendEntriesRes,
          const unsigned int version) {
  ar &htonl(appendEntriesRes.term);
  ar & appendEntriesRes.success;
}
template <typename Archive>
void save(Archive &ar, const SerializationProxy &proxy,
          const unsigned int version) {
  ar & proxy.tag;
  std::visit([&ar](auto &&rest) { ar & rest; }, proxy.rest);
}
} // namespace serialization
} // namespace boost
BOOST_SERIALIZATION_SPLIT_FREE(AppendEntries)
BOOST_SERIALIZATION_SPLIT_FREE(AppendEntriesResponse)
BOOST_SERIALIZATION_SPLIT_FREE(RequestVote)
BOOST_SERIALIZATION_SPLIT_FREE(RequestVoteResponse)
BOOST_SERIALIZATION_SPLIT_FREE(SerializationProxy)

static std::unique_ptr<asio::streambuf> serialize_internal(RPCType message) {
  auto index = message.index();
  SerializationProxy proxy{static_cast<uint8_t>(index), std::move(message)};
  auto buf = std::make_unique<asio::streambuf>();
  std::ostream os(&(*buf));
  boost::archive::binary_oarchive oa{os};
  oa << proxy;
  return buf;
}

void serialize(RequestType request) {
  serialize_internal(variant_cast(std::move(request)));
}

void serialize(ResponseType response) {
  serialize_internal(variant_cast(std::move(response)));
}

std::optional<RPCType> deserialize_internal() { return std::nullopt; }

std::optional<MessageType> deserialize() {
  auto v = deserialize_internal();
  if (!v) {
    return std::nullopt;
  }

  return std::visit(
      [](auto &&v) {
        if constexpr (variant_contains<RequestType, decltype(v)>()) {
          return std::variant<RequestType>{std::forward<decltype(v)>(v)};
        } else if constexpr (variant_contains<ResponseType, decltype(v)>()) {
          return std::variant<ResponseType>{std::forward<decltype(v)>(v)};
        } else {
          return std::nullopt;
        }
      },
      *v);
}
