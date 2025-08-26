/**
* @file condition_variable.hpp
* @author daguai
* @version 1.0
*/

#pragma once

#include <condition_variable>
#include <functional>
#include <random>

#include "config.h"
#include "coro/attribute.hpp"
#include "coro/comp/mutex.hpp"
#include "coro/context.hpp"
#include "coro/spinlock.hpp"

#pragma once

#include <functional>

#include "coro/attribute.hpp"
#include "coro/comp/mutex.hpp"
#include "coro/spinlock.hpp"

namespace coro
{

using cond_type = std::function<bool()>;

class condition_variable;
using cond_var = condition_variable;

class condition_variable final
{
public:
    struct cv_awaiter : public mutex::mutex_awaiter
    {
        friend condition_variable;

        cv_awaiter(context& ctx, mutex& mtx, cond_var& cv) noexcept
            : mutex_awaiter(ctx, mtx),
              m_cv(cv),
              m_suspend_state(false)
        {
        }
        cv_awaiter(context& ctx, mutex& mtx, cond_var& cv, cond_type& cond) noexcept
            : mutex_awaiter(ctx, mtx),
              m_cv(cv),
              m_cond(cond),
              m_suspend_state(false)
        {
        }

        auto await_suspend(std::coroutine_handle<> handle) noexcept -> bool;

        auto await_resume() noexcept -> void;

    protected:
        auto register_lock() noexcept -> bool;

        auto register_cv() noexcept -> void;

        auto wake_up() noexcept -> void;

        auto resume() noexcept -> void override;

        cond_type m_cond;  // 谓词
        cond_var& m_cv;    // 绑定的condition_variable
        bool      m_suspend_state; // 挂起的状态
    };

public:
    condition_variable() noexcept = default;
    ~condition_variable() noexcept;

    CORO_NO_COPY_MOVE(condition_variable);

    auto wait(mutex& mtx) noexcept -> cv_awaiter;

    auto wait(mutex& mtx, cond_type&& cond) noexcept -> cv_awaiter;

    auto wait(mutex& mtx, cond_type& cond) noexcept -> cv_awaiter;

    auto notify_one() noexcept -> void;

    auto notify_all() noexcept -> void;

private:
    detail::spinlock m_lock;
    alignas(config::kCacheLineSize) cv_awaiter* m_head{nullptr};
    alignas(config::kCacheLineSize) cv_awaiter* m_tail{nullptr};
};

}; // namespace coro

