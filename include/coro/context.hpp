#pragma once

#include <atomic>
#include <coroutine>
#include <functional>
#include <memory>
#include <thread>

#include "config.h"
#include "coro/engine.hpp"
#include "coro/meta_info.hpp"
#include "coro/task.hpp"

namespace coro
{
using config::ctx_id;
using std::atomic;
using std::make_unique;
using std::memory_order_acq_rel;
using std::memory_order_acquire;
using std::memory_order_relaxed;
using std::stop_token;
using std::unique_ptr;
using std::jthread;

using detail::ginfo;
using detail::linfo;

using engine = detail::engine;

class scheduler;

class context
{
    using stop_cb = std::function<void()>;

public:
    context() noexcept;
    ~context() noexcept                = default;
    context(const context&)            = delete;
    context(context&&)                 = delete;
    context& operator=(const context&) = delete;
    context& operator=(context&&)      = delete;

    /**
    * @brief  启动工作线程
    */
    auto start() noexcept -> void;

    /**
    * @brief 发送停止信号到工作线程
    */
    auto notify_stop() noexcept -> void;

    /**
    * @brief 等待工作线程停止
    */
    inline auto join() noexcept -> void { m_job->join(); }

    /**
    * @brief 设置停止毁掉函数对象
    *
    * @param cb
    */
    auto set_stop_cb(stop_cb cb) noexcept -> void;

    /**
     * @brief 将task的生命周期交给engine去管理
     * @param task&&
     */
    inline auto submit_task(task<void>&& task) noexcept -> void
    {
        auto handle = task.handle();
        task.detach();
        this->submit_task(handle);
    }

    /**
     * @brief task的生命周期由调用者去管理
     * @param task&
     */
    inline auto submit_task(task<void>& task) noexcept -> void { submit_task(task.handle()); }

    /**
    * @brief 提交一个任务句柄到context
    */
    inline auto submit_task(std::coroutine_handle<> handle) noexcept -> void
    {
        m_engine.submit_task(handle);
    }

    /**
     * @brief  get context unique id
     *
     * @return ctx_id
     */
    inline auto get_ctx_id() noexcept -> ctx_id { return m_id; }

    /**
    * @brief 添加context的任务计数
    *
    * @param  register_cnt
    */
    inline auto register_wait(int register_cnt = 1) noexcept -> void
    {
        m_num_wait_task.fetch_add(size_t(register_cnt), memory_order_acq_rel);
    }

    /**
    * @brief 减少context的任务计数
    *
    * @param  register_cnt
    */
    inline auto unregister_wait(int register_cnt = 1) noexcept -> void
    {
        m_num_wait_task.fetch_sub(register_cnt, memory_order_acq_rel);
    }

    inline auto get_engine() noexcept -> engine& { return m_engine; }

    auto init() noexcept -> void;

    auto deinit() noexcept -> void;

    /**
     * @brief 工作线程的主逻辑
     * @param token
     */
    auto run(stop_token token) noexcept -> void;

    auto process_work() noexcept -> void;

    inline auto poll_work() noexcept -> void { m_engine.poll_submit(); }


    /**
    * @brief 判断引用计数是否为0
    *
    * @return true
    * @return false
    */
    inline auto empty_wait_task() noexcept -> bool CORO_INLINE
    {
        return m_num_wait_task.load(memory_order_acquire) == 0 && m_engine.empty_io();
    }


private:
    CORO_ALIGN engine   m_engine; // 该context拥有的 engine
    unique_ptr<jthread> m_job;  // 工作线程
    ctx_id              m_id;    //唯一id
    atomic<size_t>      m_num_wait_task{0}; // 等待中的任务数量(引用计数)
    stop_cb             m_stop_cb; // 停止时的回调函数
};

inline context& local_context() noexcept
{
    return *linfo.ctx;
}

inline void submit_to_context(task<void>&& task) noexcept
{
    local_context().submit_task(std::move(task));
}

inline void submit_to_context(task<void>& task) noexcept
{
    local_context().submit_task(task.handle());
}

inline void submit_to_context(std::coroutine_handle<> handle) noexcept
{
    local_context().submit_task(handle);
}


}; // namespace coro
