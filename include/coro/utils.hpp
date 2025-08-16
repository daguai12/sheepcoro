#pragma once

#include <chrono>
#include <cstdint>
#include <regex>
#include <string>
#include <thread>

namespace coro::utils 
{
    
auto get_null_fd() noexcept -> int;

}; //namespace coro::utils
