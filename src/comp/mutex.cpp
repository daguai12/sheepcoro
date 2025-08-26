#include "coro/comp/mutex.hpp"
#include "coro/context.hpp"
#include "coro/scheduler.hpp"
#include <atomic>
#include <coroutine>

namespace coro 
{
auto mutex::mutex_awaiter::await_resume() noexcept -> void
{
    m_ctx.unregister_wait();
}

auto mutex::mutex_awaiter::await_suspend(std::coroutine_handle<> handle) noexcept -> bool
{
    m_await_coro = handle;
    m_ctx.register_wait();
    return register_lock();
}

auto mutex::mutex_awaiter::register_lock() noexcept -> bool
{
    while (true)
    {
        auto state = m_mtx.m_state.load(std::memory_order_acquire);
        m_next = nullptr;
        if (state == mutex::nolocked)
        {
            if (m_mtx.m_state.compare_exchange_weak(state, mutex::locked_no_waiting,std::memory_order_acq_rel,std::memory_order_relaxed))
            {
                return false;
            }
        }
        else
        {
            m_next = reinterpret_cast<mutex_awaiter*>(state);
            if (m_mtx.m_state.compare_exchange_weak(
                    state, reinterpret_cast<awaiter_ptr>(this), std::memory_order_acq_rel, std::memory_order_relaxed))
            {
                // m_ctx.register_wait(m_register_state);
                // m_register_state = false;
                return true;
            }
        }
    }
}

auto mutex::mutex_awaiter::resume() noexcept -> void
{
    m_ctx.submit_task(m_await_coro);
}

auto mutex::try_lock() noexcept -> bool
{
    auto target = nolocked;
    return m_state.compare_exchange_strong(target, locked_no_waiting, std::memory_order_acq_rel, std::memory_order_relaxed);
}

auto mutex::lock() noexcept -> mutex_awaiter
{
    return mutex_awaiter(local_context(),*this);
}

auto mutex::unlock() noexcept -> void
{
    assert(m_state.load(std::memory_order_acquire) != nolocked && "unlock the mutex with unlock state");

    auto to_resume = reinterpret_cast<mutex_awaiter*>(m_resume_list_head);
    if (to_resume == nullptr)
    {
        auto target = locked_no_waiting;
        if (m_state.compare_exchange_strong(target, nolocked, std::memory_order_acq_rel, std::memory_order_relaxed))
        {
            return; // no waiters and we set to unlocked
        }

        auto head = m_state.exchange(locked_no_waiting, std::memory_order_acq_rel);
        assert(head != nolocked && head != locked_no_waiting);

        auto awaiter = reinterpret_cast<mutex_awaiter*>(head);
        do 
        {
            auto temp = awaiter->m_next;
            awaiter->m_next            = to_resume;
            to_resume                  = awaiter;
            awaiter                    = temp;
        } while(awaiter != nullptr);
    }

    assert(to_resume != nullptr && "unexpected to_resume value:nullptr");
    m_resume_list_head = to_resume->m_next;
    to_resume->resume();
}

auto mutex::lock_guard() noexcept -> mutex_guard_awaiter
{
    return mutex_guard_awaiter(local_context(),*this);
}
    
}; // namespace coro
