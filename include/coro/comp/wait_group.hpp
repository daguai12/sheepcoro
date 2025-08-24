/**
* @file wait_group.hpp
* @author daguai
* @version 1.0
*/

#pragma once

#include <atomic>
#include <coroutine>

#include "coro/detail/types.hpp"

namespace coro
{
class context;
using detail::awaiter_ptr;

class wait_group
{
public:
    struct awaiter
    {
        awaiter(context& ctx, wait_group& wg) noexcept : m_ctx(ctx), m_wg(wg) {}

        constexpr auto await_ready() noexcept -> bool { return false; }

        auto await_suspend(std::coroutine_handle<> handle) noexcept -> bool;

        auto await_resume() noexcept -> void;

        auto resume() noexcept -> void;

        context&                m_ctx; // 等待者所在的context
        wait_group&             m_wg;  // 关联的wait_group
        awaiter*                m_next{nullptr}; // 链表next
        std::coroutine_handle<> m_await_coro{nullptr}; // 恢复用的协程句柄
    };

    explicit wait_group(int count = 0) noexcept : m_count(count) {}

    auto add(int count) noexcept -> void;

    auto done() noexcept -> void;

    auto wait() noexcept -> awaiter;

private:
    friend awaiter;
    std::atomic<int32_t>     m_count; // 剩余任务数
    std::atomic<awaiter_ptr> m_state; // 等待者链表的头指针
};

}; // namespace coro
