#include "coro/engine.hpp"
#include "coro/meta_info.hpp"
#include "coro/uring_proxy.hpp"

namespace coro::io::detail 
{
    
struct fixed_fds
{
    fixed_fds() noexcept
    {
        // 从 uring_proxy 维护的注册文件槽池里借一个空槽
        item = ::coro::detail::local_engine().get_uring().get_fixed_fd();
    }

    ~fixed_fds() noexcept { return_back(); }

    inline auto assign(int& fd, int &flag) noexcept -> void
    {
        if (!item.valid())
        {
            return;
        }
        *(item.ptr) = fd;
        fd          = item.idx;
        flag |= IOSQE_FIXED_FILE;
        ::coro::detail::local_engine().get_uring().update_register_fixed_fds(item.idx);
    }

    inline auto return_back() noexcept -> void
    {
        // 归还fixed fd给uring
        if (item.valid())
        {
            ::coro::detail::local_engine().get_uring().back_fixed_fd(item);
            item.set_invalid();
        }
    }

    ::coro::uring::uring_fds_item item;
};
}; // namespace coro::io::detail
