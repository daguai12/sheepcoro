#include "coro/comp/event.hpp"
#include "coro/scheduler.hpp"

namespace coro::detail
{
auto event_base::awaiter_base::await_ready() noexcept -> bool
{
    m_ctx.register_wait();
    return m_ev.is_set();
}

auto event_base::awaiter_base::await_suspend(std::coroutine_handle<> handle) noexcept -> bool
{
    m_await_coro = handle;
    return m_ev.register_awaiter(this);
}

auto event_base::awaiter_base::await_resume() noexcept -> void
{
    m_ctx.unregister_wait();
}

auto event_base::set_state() noexcept -> void
{
    auto flag = m_state.exchange(this, std::memory_order_acq_rel);
    if (flag != this)
    {
        auto waiter = static_cast<awaiter_base*>(flag);
        resume_all_awaiter(waiter);
    }
}

auto event_base::resume_all_awaiter(awaiter_ptr waiter) noexcept -> void
{
    while (waiter != nullptr)
    {
        auto cur = static_cast<awaiter_base*>(waiter);
        cur->m_ctx.submit_task(cur->m_await_coro);
        // cur->m_ctx.unregister_wait();
        waiter = cur->m_next;
    }
}

auto event_base::register_awaiter(awaiter_base* waiter) noexcept -> bool
{
    const auto  set_state = this;
    awaiter_ptr old_value = nullptr;

    do
    {
        old_value = m_state.load(std::memory_order_acquire);
        if (old_value == this)
        {
            waiter->m_next = nullptr;
            return false;
        }
        waiter->m_next = static_cast<awaiter_base*>(old_value);
    } while (!m_state.compare_exchange_weak(old_value, waiter, std::memory_order_acquire));

    // waiter->m_ctx.register_wait();
    return true;
}

}; // namespace coro::detail
