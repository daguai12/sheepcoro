#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "coro/context.hpp"
#include "coro/detail/types.hpp"

namespace coro::detail
{
using ctx_container = std::vector<std::unique_ptr<context>>;

template<dispatch_strategy dst>
class dispatcher
{
public:

    void init([[CORO_MAYBE_UNUSED]] const size_t ctx_cnt, [[CORO_MAYBE_UNUSED]] ctx_container* ctxs) noexcept {}

    auto dispatch() noexcept -> size_t { return 0; }
};


/**
* @brief 采用round robin的方式将任务分发给context
*
* @tparam
*/
template<>
class dispatcher<dispatch_strategy::round_robin>
{
public:
    void init(size_t ctx_cnt, [[CORO_MAYBE_UNUSED]] ctx_container* ctxs) noexcept
    {
        m_ctx_cnt = ctx_cnt;
        m_cur     = 0;
    }

    auto dispatch() noexcept -> size_t { return m_cur.fetch_add(1,std::memory_order_acq_rel) % m_ctx_cnt; }
    
private:
    size_t              m_ctx_cnt;
    std::atomic<size_t> m_cur{0};
};

}; // namespace coro::detail 
