#pragma once

#include <atomic>

namespace coro::detail
{
using std::atomic;
using std::memory_order_acquire;
using std::memory_order_relaxed;
using std::memory_order_release;

struct spinlock
{
    atomic<bool> lock_ = {0};

    void lock() noexcept
    {
        for (;;)
        {
            if (!lock_.exchange(true, memory_order_acquire))
            {
                return;
            }

            while (lock_.load(memory_order_relaxed))
            {
                #if defined(__GUNC__) || defined(__clang__)
                    __builtin_ia32_pause();
                #elif defined(_MSC_VER)
                     __mm_pause();
                #else
                    __builtin_ia32_pause();
                #endif
            }
        }
    }

    bool try_lock() noexcept
    {
        return !lock_.load(memory_order_relaxed) && !lock_.exchange(true, memory_order_acquire);
    }


    void unlock() noexcept { lock_.store(false, memory_order_release); }

};


}; // namespace coro 
