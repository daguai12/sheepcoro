#include "coro/coro.hpp"
#include "coro/scheduler.hpp"

using namespace coro;
using coro::time::timer;

task<> timer_func()
{
    log::info("timer begin");
    auto result = co_await timer().add_seconds(2);
    log::info("timer end, result: {}", result);
}

int main()
{
    scheduler::init();
    submit_to_scheduler(timer_func());
    scheduler::loop();
    return 0;
}



