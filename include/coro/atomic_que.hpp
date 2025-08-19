#include <cstddef>

#include "config.h"

#include "atomic_queue/atomic_queue.h"

namespace coro::detail 
{
// lib AtomicQueue
template <class Queue, size_t Capacity>
struct CapacityToConstructor : Queue
{
    CapacityToConstructor() : Queue(Capacity) {}
};


/**
* @brief 多生产者多消费者 (mpmc) 原子队列
* 
* @tparam T 存储的数据类型
*
* @note 模板参数含义： true, false, false
*       其中 MAXIMIZE_THROUGHPUT = true(最大吞吐量)
*       TOTAL_ORDER = false (不保证全局顺序)
*       bool SPSC = false (非单生产者单消费者模式)
*/
template<typename T>
using AtomicQueue = 
    CapacityToConstructor<atomic_queue::AtomicQueueB2<T, std::allocator<T>, true, false, false>, config::kQueCap>;
}
