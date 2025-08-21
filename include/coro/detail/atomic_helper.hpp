#include <atomic>

#include "config.h"

namespace coro::detail 
{
/**
* @brief 使用std::vector<atomic<T>>会造成编译错误，所以使用atomic_ref_wrapper
*
* @tparam T
*/
template <typename T>
struct alignas(config::kCacheLineSize) atomic_ref_wrapper
{
    alignas(std::atomic_ref<T>::required_alignment) T val;
};

}; // namespace coro::detail
