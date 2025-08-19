#pragma once

#include <atomic>

#include "coro/attribute.hpp"

namespace coro 
{
class context;
}; // namespace coro 

namespace coro::detail 
{
using config::ctx_id;
using std::atomic;
class engine;

struct CORO_ALIGN local_info
{
    // 存储协程句柄
    context* ctx{nullptr}; 
    // 存储执行引擎engine
    engine*  egn{nullptr};
};


struct global_info
{
    atomic<ctx_id>   context_id{0};
    atomic<uint32_t> engine_id{0};
};

// TLS(Thread local store)
inline thread_local local_info linfo;
// global varibale
inline global_info             ginfo;

// init global info
inline auto init_meta_info() noexcept -> void
{
    ginfo.context_id = 0;
    ginfo.engine_id  = 0;
}

// 判断当前线程线程是否工作线程
inline auto is_in_working_state() noexcept -> bool
{
    return linfo.ctx != nullptr;
}
}; // namespace coro::detail
