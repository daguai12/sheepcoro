#include "coro/comp/wait_group.hpp"
#include "coro/context.hpp"
#include "coro/scheduler.hpp"
#include <atomic>
#include <coroutine>

namespace coro
{

auto wait_group::awaiter::await_suspend(std::coroutine_handle<> handle) noexcept -> bool
{
    m_await_coro = handle;
    m_ctx.register_wait();
    while (true)
    {
        if (m_wg.m_count.load(std::memory_order_acquire) == 0)
        {
            return false;
        }
        auto head = m_wg.m_state.load(std::memory_order_acquire);
        m_next  = static_cast<awaiter*>(head);
        if (m_wg.m_state.compare_exchange_weak(head, static_cast<awaiter_ptr>(this), std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            return true;
        }
    }
}

auto wait_group::awaiter::await_resume() noexcept -> void
{
    m_ctx.unregister_wait();
}

auto wait_group::awaiter::resume() noexcept -> void
{
    m_ctx.submit_task(m_await_coro);
}

auto wait_group::add(int count) noexcept -> void
{
    m_count.fetch_add(count, std::memory_order_acq_rel);
}

auto wait_group::done() noexcept -> void
{
    if (m_count.fetch_sub(1,std::memory_order_acq_rel) == 1)
    {
        auto head = static_cast<awaiter*>(m_state.exchange(nullptr,std::memory_order_acq_rel));
        while (head != nullptr)
        {
            head->resume();
            head = head->m_next;
        }
    }
}

auto wait_group::wait() noexcept -> awaiter
{
    return awaiter(local_context(),*this);
}

}; // namespace coro
