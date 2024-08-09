#pragma once

template <typename... Args> std::weak_ptr<raft> raft::create(Args &&...args) {
  auto service =
      std::make_shared<raft>(secret_code{}, std::forward<Args>(args)...);
  service->start_accept();
  service->process();
  return service;
}
