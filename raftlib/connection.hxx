#pragma once

#include "connection_interface.hxx"

template <typename direction>
struct connection : public std::enable_shared_from_this<connection<direction>>,
                    public connection_interface<direction> {
private:
  struct secret_code {
    explicit secret_code() = default;
  };

public:
  connection(secret_code, direction &&, asio::io_context &,
             std::shared_ptr<raft>, connection_type);

  template <typename Dir, typename... Args>
  static std::weak_ptr<connection<direction>> create(Dir &&, Args &&...);

  std::optional<asio::ip::tcp::endpoint> get_endpoint() const override;

private:
  auto shared_from_this() {
    return std::enable_shared_from_this<
        connection<direction>>::shared_from_this();
  }

  void start(const outgoing &);
  void start(const incoming &);

  connection_type parameters;
  std::shared_ptr<raft> node;
  asio::ip::tcp::socket socket;
  asio::io_context &ctx;
  direction dir;
};

#include "detail/connection.hxx"
