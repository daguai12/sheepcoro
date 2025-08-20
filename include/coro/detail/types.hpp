#pragma once

#include <cstdint>

namespace coro::detail
{

enum class schedule_strategy : uint8_t
{
    fifo, //NOTE: default
    lifo,
    none
};

enum class dispatch_strategy : uint8_t
{
    round_robin,
    none
};


using awaiter_ptr = void*;


}; // namespace coro::detail
