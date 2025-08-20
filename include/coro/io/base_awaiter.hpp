#pragma once

#include <coroutine>

#include "coro/context.hpp"
#include "coro/engine.hpp"
#include "coro/io/io_info.hpp"
#include "coro/uring_proxy.hpp"

namespace coro::io::detail 
{

class base_io_awaiter
{
public:
    base_io_awaiter() noexcept
    {
        auto& engine = coro::detail::local_engine();
        // 尝试获取一个空闲的提交槽
        m_urs = engine.get_free_urs();

        // 如果获取失败，说明提交已满
        if (m_urs == nullptr)
        {
            auto& proxy = engine.get_uring();

            while ((m_urs = engine.get_free_urs()) == nullptr)
            {
                proxy.submit();
            }
        }
    }
    

    constexpr auto await_ready() noexcept -> bool { return false; }

    auto await_suspend(std::coroutine_handle<> handle) noexcept -> void { m_info.handle = handle; }
    
    auto await_resume() noexcept -> int32_t { return m_info.result; }

protected:
    io_info             m_info;
    coro::uring::ursptr m_urs;
};
    
}; // namespace coro::io::detail
