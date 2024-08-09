#pragma once

#include <asio/executor_work_guard.hpp>
#include <asio/io_service.hpp>
#include <asio/signal_set.hpp>

struct concurrency_type;
namespace async_utils {
std::optional<asio::executor_work_guard<asio::io_context::executor_type>>
setup_work(asio::io_context &, const concurrency_type &);
std::unique_ptr<asio::signal_set> setup_signals(
    asio::io_context &,
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>>
        &&);
} // namespace async_utils
