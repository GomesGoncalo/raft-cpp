#pragma once

template <typename execution_context, typename direction>
connection<execution_context, direction>::connection(
    secret_code, direction &&dir, std::shared_ptr<raft<execution_context>> node,
    raft_options::parameters_type::connection_type parameters)
    : socket{node->get_executor()}, node{node},
      parameters{std::move(parameters)}, dir{std::forward<direction>(dir)} {}

template <typename execution_context, typename direction>
void connection<execution_context, direction>::start(const outgoing &) {
  SPDLOG_INFO("Connecting to {}", boost::lexical_cast<std::string>(dir.endpt));
  auto th = shared_from_this();
  socket.async_connect(dir.endpt, [th, this](const asio::error_code &ec) {
    if (!!ec) {
      node->on_disconnected(shared_from_this());
      return;
    }
    SPDLOG_INFO("got connection {}", ec.message());
    node->on_connected(shared_from_this());
  });
}

template <typename execution_context, typename direction>
void connection<execution_context, direction>::start(const incoming &) {
  auto th = shared_from_this();
  dir.acceptor.async_accept(socket, [this, th](const asio::error_code &ec) {
    if (!!ec) {
      SPDLOG_INFO("acceptor error: {}", ec.message());
      return;
    }

    SPDLOG_INFO("got connection from {}",
                boost::lexical_cast<std::string>(socket.remote_endpoint()));
    connection<execution_context, incoming>::create(incoming{dir.acceptor},
                                                    node, parameters);
  });
}

template <typename execution_context, typename direction>
std::optional<asio::ip::tcp::endpoint>
connection<execution_context, direction>::get_endpoint() const {
  if constexpr (std::is_same_v<direction, outgoing>) {
    return dir.endpt;
  } else {
    try {
      return socket.remote_endpoint();
    } catch (...) {
      return std::nullopt;
    }
  }
}

template <typename execution_context, typename direction>
template <typename Dir, typename... Args>
std::weak_ptr<connection<execution_context, direction>>
connection<execution_context, direction>::create(Dir &&dir, Args &&...args) {
  auto service = std::make_shared<connection<execution_context, direction>>(
      secret_code{}, std::forward<Dir>(dir), std::forward<Args>(args)...);
  static_assert(std::is_same_v<direction, Dir>,
                "direction must be the same as the one defined in the class");
  service->start(service->dir);
  return service;
}
