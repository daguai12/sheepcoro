#pragma once

#include <atomic>
#include <coroutine>
#include <memory>
#include <sched.h>
#include <thread>
#include <vector>

#include "config.h"
#include "config.h.in"
#include "coro/detail/atomic_helper.hpp"
#include "coro/dispatcher.hpp"

namespace coro
{

class scheduler
{
    friend context;
    using stop_token_type = std::atomic<int>;
    using stop_flag_type  = std::vector<detail::atomic_ref_wrapper<int>>;
public:
    inline static auto init(size_t ctx_cnt = std::thread::hardware_concurrency()) noexcept -> void
    {
        if (ctx_cnt == 0)
        {
            ctx_cnt = std::thread::hardware_concurrency();
        }
        get_instance()->init_impl(ctx_cnt);
    }

    /**
    * @brief 轮询直到所有的context完成工作
    */
    inline static auto loop() noexcept -> void { get_instance()->loop_impl(); }

    static inline auto submit(task<void>&& task) noexcept -> void
    {
        auto handle = task.handle();
        task.detach();
        submit(handle);
    }

    static inline auto submit(task<void>& task) noexcept -> void { submit(task.handle()); }

    inline static auto submit(std::coroutine_handle<> handle) noexcept -> void
    {
        get_instance()->submit_task_impl(handle);
    }

private:
    static auto get_instance() noexcept -> scheduler*
    {
        static scheduler sc;
        return &sc;
    }

    auto init_impl(size_t ctx_cnt) noexcept -> void;

    auto start_impl() noexcept -> void;

    auto loop_impl() noexcept -> void;

    auto stop_impl() noexcept -> void;

    auto submit_task_impl(std::coroutine_handle<> handle) noexcept -> void;
private:
    // 上下文数量
    size_t                m_ctx_cnt{0};
    // 存放context的容器
    detail::ctx_container m_ctxs;

    // 选择目标context的策略
    detail::dispatcher<coro::config::kDispatchStrategy> m_dispatcher;

    // 存储每一个context的状态(空闲/忙)
    stop_flag_type m_ctx_stop_flag;

    // 全局计数器,为0时可以停止scheduler
    stop_token_type m_stop_token;
};

inline void submit_to_scheduler(task<void>&& task) noexcept
{
    scheduler::submit(std::move(task));
}

inline void submit_to_scheduler(task<void>& task) noexcept
{
    scheduler::submit(task.handle());
}

inline void submit_to_scheduler(std::coroutine_handle<> handle) noexcept
{
    scheduler::submit(handle);
}

}; // namespace coro
