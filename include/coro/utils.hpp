#pragma once

#include <chrono>
#include <cstdint>
#include <regex>
#include <string>
#include <thread>

namespace coro::utils 
{
    
auto get_null_fd() noexcept -> int;

inline auto sleep(int64_t t) noexcept -> void
{
    std::this_thread::sleep_for(std::chrono::seconds(t));
}

inline auto msleep(int64_t t) noexcept -> void
{
    std::this_thread::sleep_for(std::chrono::milliseconds(t));
}

inline auto usleep(int64_t t) noexcept -> void
{
    std::this_thread::sleep_for(std::chrono::microseconds(t));
}



}; //namespace coro::utils
