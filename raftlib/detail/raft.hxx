#pragma once

struct EntryPoint {};
template <> void raft::process_state<follower, EntryPoint>();

template <typename... Args> std::weak_ptr<raft> raft::create(Args &&...args) {
  auto service =
      std::make_shared<raft>(secret_code{}, std::forward<Args>(args)...);
  service->start_accept();
  service->template process_state<follower, EntryPoint>();
  return service;
}
