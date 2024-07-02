#pragma once

template <typename direction>
connection<direction>::connection(secret_code, direction &&dir,
                                  std::shared_ptr<raft> node,
                                  connection_type parameters)
    : socket{node->get_executor()}, node{node},
      parameters{std::move(parameters)}, dir{std::forward<direction>(dir)} {}

template <typename direction>
void connection<direction>::start(const outgoing &) {
  auto th = shared_from_this();
  socket.async_connect(dir.endpt, [th, this](const asio::error_code &ec) {
    if (!!ec) {
      node->on_disconnected(shared_from_this());
      return;
    }
    node->on_connected(shared_from_this());
  });
}

template <typename direction>
void connection<direction>::start(const incoming &) {
  auto th = shared_from_this();
  dir.acceptor.async_accept(socket, [this, th](const asio::error_code &ec) {
    if (!!ec) {
      return;
    }

    connection<incoming>::create(incoming{dir.acceptor}, node, parameters);
  });
}

template <typename direction>
std::optional<asio::ip::tcp::endpoint>
connection<direction>::get_endpoint() const {
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

template <typename direction>
template <typename Dir, typename... Args>
std::weak_ptr<connection<direction>>
connection<direction>::create(Dir &&dir, Args &&...args) {
  auto service = std::make_shared<connection<direction>>(
      secret_code{}, std::forward<Dir>(dir), std::forward<Args>(args)...);
  static_assert(std::is_same_v<direction, Dir>,
                "direction must be the same as the one defined in the class");
  service->start(service->dir);
  return service;
}
