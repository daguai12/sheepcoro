
#include <cassert>
#include <fcntl.h>

#include "coro/utils.hpp"

namespace coro::utils 
{
    

auto get_null_fd() noexcept -> int
{
    auto fd = open("/dev/null", O_RDWR);
    return fd;
}


}; // namespace coro::utils 
