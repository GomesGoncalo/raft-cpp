#pragma once

#include "raft_options.hxx"
#include <asio/ip/tcp.hpp>

struct acceptor : private asio::ip::tcp::acceptor {
  acceptor(asio::io_context &, const parameters_type &);

  asio::ip::tcp::acceptor &get() { return *this; }
  using asio::ip::tcp::acceptor::close;
};
