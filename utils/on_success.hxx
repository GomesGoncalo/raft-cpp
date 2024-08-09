#pragma once

std::invocable<const asio::error_code &> decltype(auto)
on_success(std::invocable auto &&callback) {
  return [callback = std::forward<decltype(callback)>(callback)](
             const asio::error_code &ec) mutable {
    if (ec) {
      return;
    }

    std::invoke(std::forward<decltype(callback)>(callback));
  };
}
