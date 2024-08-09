#pragma once

#include <asio/steady_timer.hpp>
#include <utils/chrono.hxx>

template <typename ChronoT>
  requires(ChronoDuration<ChronoT>)
void execute_after(asio::steady_timer &timer, const ChronoT &time,
                   std::invocable<const asio::error_code &> auto &&callback) {
  timer.expires_after(time);
  timer.async_wait(std::forward<decltype(callback)>(callback));
}
