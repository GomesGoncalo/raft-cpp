#pragma once

#include "acceptor.hxx"
#include <asio/ip/tcp.hpp>

struct incoming {
  acceptor &accept;
};
struct outgoing {
  asio::ip::tcp::endpoint endpt;
};

template <typename direction> struct connection_interface {
  virtual ~connection_interface() = default;
  virtual std::optional<asio::ip::tcp::endpoint> get_endpoint() const = 0;
};
