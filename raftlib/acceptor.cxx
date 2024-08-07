#include "acceptor.hxx"
#include "raft_options.hxx"

acceptor::acceptor(asio::io_context &ctx, const parameters_type &params)
    : asio::ip::tcp::acceptor(ctx) {
  open(params.bind.protocol());
  set_option(asio::ip::tcp::acceptor::reuse_address(true));
  bind(params.bind);
  listen();
}
