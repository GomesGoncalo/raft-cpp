#include "message.hxx"
#include "detail/message_sparse.hxx"
#include <arpa/inet.h>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

static std::unique_ptr<asio::streambuf> serialize_internal(RPCType message) {
  auto buf = std::make_unique<asio::streambuf>();

  /*
  auto index = message.index();
  SerializationProxy proxy{static_cast<uint8_t>(index), std::move(message)};
  std::ostream os(&(*buf));
  boost::archive::binary_oarchive oa{os};
  oa << proxy;
  */

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
