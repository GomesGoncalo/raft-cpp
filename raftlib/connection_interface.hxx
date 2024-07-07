#pragma once

#include <asio/ip/tcp.hpp>

struct incoming {
  asio::ip::tcp::acceptor &acceptor;
};
struct outgoing {
  asio::ip::tcp::endpoint endpt;
};

template <typename direction> struct connection_interface {
  virtual ~connection_interface() = default;
  virtual std::optional<asio::ip::tcp::endpoint> get_endpoint() const = 0;
};
