#include "coro/context.hpp"
#include "coro/detail/container.hpp"
#include "coro/meta_info.hpp"
#include <atomic>
#include <optional>
#include <stop_token>
// #include "coro/sched"

namespace coro 
{
context::context() noexcept
{
    m_id = ginfo.context_id.fetch_add(1,std::memory_order_relaxed);
}

auto context::start() noexcept -> void
{
    m_job = make_unique<jthread>(
        [this](stop_token token)
        {
            this->init();
            // 如果没有scheduler接管(没有设置 stop_cb)
            // 则设置一个默认的停止回调: 调用 request_top()
            if (!(this->m_stop_cb))
            {
                m_stop_cb = [&]() {m_job->request_stop();};
            }
            this->run(token);
            this->deinit();
        });
}

auto context::notify_stop() noexcept -> void
{
    m_job->request_stop();
    m_engine.wake_up();
}

auto context::set_stop_cb(stop_cb cb) noexcept -> void
{
    m_stop_cb = cb;
}

auto context::init() noexcept -> void
{
    linfo.ctx = this;
    m_engine.init();
}

auto context::deinit() noexcept -> void
{
    linfo.ctx = nullptr;
    m_engine.deinit();
}

auto context::run(stop_token token) noexcept -> void
{
    while (!token.stop_requested())
    {
        process_work();

        if (empty_wait_task()) {
            if (!m_engine.ready()) {
                m_stop_cb();
            } else {
                continue;
            }
        }

        poll_work();
    }
}


auto context::process_work() noexcept -> void
{
    // while (m_engine.ready())
    // {
    //     m_engine.exec_one_task();
    // }

    auto num = m_engine.num_task_schedule();
    for (int i = 0; i < num; i++)
    {
        m_engine.exec_one_task();

    }
}

}; // namespace coro
