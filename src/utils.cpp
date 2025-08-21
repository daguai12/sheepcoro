
#include <cassert>
#include <fcntl.h>

#include "coro/utils.hpp"

namespace coro::utils 
{
auto set_fd_noblock(int fd) noexcept -> void
{
    int flags = fcntl(fd,F_GETFL,0);
    assert(flags >= 0);

    flags |= O_NONBLOCK;
    assert(fcntl(fd, F_SETFL,flags) >= 0);
}


auto get_null_fd() noexcept -> int
{
    auto fd = open("/dev/null", O_RDWR);
    return fd;
}


}; // namespace coro::utils 
