/**
* @File engine.hpp
* @author daguai
* @version 1.0
*/
#pragma once

#include <array>
#include <atomic>
#include <coroutine>
#include <cstdint>
#include <functional>
#include <queue>
#include <sys/types.h>
#include <type_traits>

#include "config.h"
#include "coro/atomic_que.hpp"
#include "coro/attribute.hpp"
#include "coro/meta_info.hpp"
#include "coro/uring_proxy.hpp"

namespace coro 
{
class context;
}; // namespace coro

namespace coro::detail 
{
using std::array;
using std::atomic;
using std::coroutine_handle;
using std::queue;
using uring::urcptr;
using uring::ursptr;
using uring::uring_proxy;

inline engine& local_engine() noexcept;

template<typename T>
using mpmc_queue = AtomicQueue<T>;

#define wake_by_task(val) (((val)&engine::task_mask) > 0)
#define wake_by_io(val)   (((val)&engine::io_mask)   > 0)
#define wake_by_cqe(val)  (((val)&engine::cqe_mask)  > 0)

class engine
{
    friend class ::coro::context;

public:
    static constexpr uint64_t task_mask = (0xFFFFF00000000000);
    static constexpr uint64_t io_mask   = (0x00000FFFFF000000);
    static constexpr uint64_t cqe_mask  = (0x0000000000FFFFFF);

    static constexpr uint64_t task_flag = (((uint64_t)1) << 44);
    static constexpr uint64_t io_flag   = (((uint64_t)1) << 24);

    engine() noexcept : m_num_io_wait_submit(0), m_num_io_running(0) 
    {
        m_id = ginfo.engine_id.fetch_add(1, std::memory_order_relaxed);
    }

    ~engine() noexcept = default;

    // 禁止engine的copy和move,保证engine instance唯一
    engine(const engine&)                    = delete;
    engine(engine&&)                         = delete;
    auto operator=(const engine&) -> engine& = delete;
    auto operator=(engine&&) -> engine&      = delete;

    auto init() noexcept -> void;

    auto deinit() noexcept -> void;

    /**
     * @brief 判断当前engine是否就绪
     *
     * @return true/false
     */
    inline auto ready() noexcept -> bool { return !m_task_queue.was_empty(); }

    /**
     * @brief 获取空闲sqe entry
     * 
     * @return ursptr
     */
    [[CORO_DISCARD_HINT]] inline auto get_free_urs() noexcept -> ursptr { return m_upxy.get_free_sqe(); }

    /**
     * @brief 获取在当前engine中正在运行的task
     * 
     * @return size_t
     */
    inline auto num_task_schedule() noexcept -> size_t { return m_task_queue.was_size(); }

    /**
     * @brief 从任务队列取出一个协程句柄
     *
     * @return coroutine_handle
     */
    [[CORO_DISCARD_HINT]] auto schedule() noexcept -> coroutine_handle<>;

    /**
     * @brief 提交一个task句柄到engine中
     *
     * @param handle
     */
    auto submit_task(coroutine_handle<> handle) noexcept -> void;

    /**
     * @brief 调用 schedule() 取出一个任务并执行 
     *
     * @note task执行完毕调用clean清理资源
     */
    auto exec_one_task() noexcept -> void;

    /**
     * @brief 处理单个io_uring完成事件(CQE)
     *
     * @param urcptr
     */
    auto handle_cqe_entry(urcptr cqe) noexcept -> void;

    /**
     * @brief 提交SQE并阻塞等待io_uring完成, 然后从CQE中取出条目调用 handle_cqe_entry
     */
    auto poll_submit() noexcept -> void;

    /**
     * @brief 唤醒因读取eventfd而阻塞的线程
     */
    auto wake_up(uint64_t val = engine::task_flag) noexcept -> void;

    /**
     * @brief 增加需要提交的IO
     */
    inline auto add_io_submit() noexcept -> void
    {
        m_num_io_wait_submit += 1;
    }

    /**
     * @brief 判断没有待提交的也没有正在运行的IO
     */
    inline auto empty_io() noexcept -> bool
    {
        return m_num_io_wait_submit == 0 && m_num_io_running == 0;
    }

    /**
     * @brief 返回 engine id
     */
     inline auto get_id() noexcept -> uint32_t { return m_id; }

    /**
     * @brief 返回 uring_proxy 引用
     *
     * @return uring_proxy&
     */
    inline auto get_uring() noexcept -> uring_proxy& { return m_upxy;}

private:
    /**
     * @brief 把 m_num_io_wait_submit 中记录的需要提交项写入 m_upxy 的SQE并调用submit
     */
    auto do_io_submit() noexcept -> void;

    /**
     * @brief resume协程执行 coroutine_handle的封装
     */
    auto exec_task(coroutine_handle<> handle) -> void;

    /**
     * @brief 判断当前engine是否为线程的local_engine
     */
    inline auto is_local_engine() noexcept -> bool { return get_id() == local_engine().get_id(); };

private:
    // unique engine_id
    uint32_t    m_id;
    // io_uring instance
    uring_proxy m_upxy;

    // 存储协程句柄
    mpmc_queue<coroutine_handle<>> m_task_queue;

    // 存储 CQE entry
    array<urcptr, config::kQueCap> m_urc;

    /**
    * @note io_uring官方建议使用单线程， 所以任务的提交并不会夸线程, 无需使用atomic
    */
    // 待提交的任务数量
    size_t m_num_io_wait_submit{0};
    // atomic<size_t> m_num_io_wait_submit{0};

    // 已提交正在运行的任务数量
    size_t m_num_io_running{0};
    // atomic<size_t> m_num_io_running{0};

    // stack的递归深度
    size_t m_max_recursive_depth{0};
};

/**
* @brief 返回 local thread engine
*
* @return  engine&
*/
inline engine& local_engine()  noexcept
{
    return *linfo.egn;
}

    
}; //namespace coro::detail

