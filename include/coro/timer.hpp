#pragma once

#include "coro/context.hpp"
#include "coro/engine.hpp"
#include "coro/io/io_info.hpp"
#include <chrono>
#include <cstdint>
#include <liburing.h>
#ifndef _SYS_TIME_H
    #define _SYS_TIME_H
#endif

#include "coro/attribute.hpp"
#include "coro/concepts/common.hpp"
#include "coro/io/base_awaiter.hpp"

namespace coro::time 
{

using coro_system_clock          = std::chrono::system_clock;
using coro_steady_clock          = std::chrono::steady_clock;
using coro_high_resolution_clock = std::chrono::high_resolution_clock;

#define timeout_abs           IORING_TIMEOUT_ABS
#define timeout_boottime      IORING_TIMEOUT_BOOTTIME
#define timeout_realtime      IORING_TIMEOUT_REALTIME
#define timeout_monotonic     0
#define timeout_etime_success IORING_TIMEOUT_ETIME_SUCCESS
#define timeout_multishot     IORING_TIMEOUT_MULTISHOT

namespace detail
{
/**
* @brief 从std::duration 获取内核时间 timespec 对象
*
* @tparam Rep: 时间周期的数值类型
* @tparam Period: 时间周期（如秒，毫秒等)
* @param time_duration: 时间间隔对象
* @return __kernel_timespec: 内核使用的 timespec结构体
*/
template<typename Rep, typename Period>
inline auto get_kernel_timespec(std::chrono::duration<Rep, Period> time_duration) -> __kernel_timespec
{
    auto seconds     = std::chrono::duration_cast<std::chrono::seconds>(time_duration);
    auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(time_duration - seconds);
    return __kernel_timespec{.tv_sec = seconds, .tv_nsec = nanoseconds};
}

/**
* @brief 从std::time_point 获取内核的timespec对象
* 
* @tparam Clock: 仅支持 [system_clock|steady_clock|high_resolution_clock]
* @tparam Duration: C++标准： std::duration类型
* @param time_point: 时间点对象
* @return __kernel_timespec: 内核使用的timespec结构体
*/
template<typename Clock,typename Duration>
requires(coro::concepts::in_types<
         Clock,
         coro_system_clock,
         coro_steady_clock,
         coro_high_resolution_clock>)
inline auto get_kernel_timespec(std::chrono::time_point<Clock, Duration> time_point) -> __kernel_timespec
{
    return get_kernel_timespec(time_point.time_since_epoch());
}

}; // namespace detail

using ::coro::detail::local_engine;
using coro::io::detail::io_info;
using coro::io::detail::io_type;

/**
* @brief 支持链式调用的定时器
*/
class timer
{
    struct timer_awaiter : coro::io::detail::base_io_awaiter
    {
        timer_awaiter(__kernel_timespec ts, int count, unsigned flags) noexcept
        {
            m_info.type = io_type::timer;
            m_info.cb   = &timer_awaiter::callback;
            m_ts        = ts;

            io_uring_prep_timeout(m_urs, &m_ts, count, flags);
            io_uring_sqe_set_data(m_urs, &m_info);
            local_engine().add_io_submit();
        }

        static auto callback(io_info* data, int res) noexcept -> void
        {
            // ignore timeout error
            if (res == -ETIME)
            {
                res = 0;
            }
            data->result = res;
            submit_to_context(data->handle);
        }

        // 复制传入的ts到 timer_awaiter内部m_ts 避免竞态条件
        __kernel_timespec m_ts;
    };

public:
    explicit timer(unsigned flag = timeout_monotonic) noexcept : m_flag(flag) {}

    /**
    * @brief 添加时间(秒)
    *
    * @param secnods
    * @return timer&
    */
    CORO_INLINE auto add_seconds(uint64_t seconds) -> timer&
    {
        m_ts.tv_sec += seconds;
        return *this;
    }

    /**
    * @brief 添加时间(毫秒)
    *
    * @param secnods
    * @return timer&
    */
    CORO_INLINE auto add_mseconds(uint64_t mseconds) -> timer&
    {
        m_ts.tv_nsec += (1000000 * mseconds);
        return *this;
    }

    /**
    * @brief 添加时间(微秒)
    *
    * @param secnods
    * @return timer&
    */
    CORO_INLINE auto add_useconds(uint64_t useconds) -> timer&
    {
        m_ts.tv_nsec += (1000 * useconds);
        return *this;
    }

    /**
    * @brief 添加时间(纳秒)
    *
    * @param secnods
    * @return timer&
    */
    CORO_INLINE auto add_nseconds(uint64_t nseconds) -> timer&
    {
        m_ts.tv_nsec += nseconds;
        return *this;
    }

    template<typename Rep, typename Period>
    auto set_by_duration(std::chrono::duration<Rep, Period> time_duration) -> timer&
    {
        m_ts = detail::get_kernel_timespec(time_duration);
        return *this;
    }

    template<typename Clock, typename Duration>
    requires(coro::concepts::in_types<
             Clock, 
             coro_steady_clock,
             coro_system_clock,
             coro_high_resolution_clock>)
    auto set_by_timepoint(std::chrono::time_point<Clock, Duration> time_point) -> timer&
    {
        m_ts = detail::get_kernel_timespec(time_point);
        return *this;
    }

    auto operator co_await()  noexcept
    {
        return timer_awaiter(m_ts,1,0);
    }

private:
    __kernel_timespec m_ts;
    unsigned          m_flag{0};
};
}; // namespace coro::time
