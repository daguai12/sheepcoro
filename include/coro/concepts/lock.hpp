#pragma once

#include <concepts>

namespace coro::concepts
{
template<typename T>
concept locktype = requires(T mtx) {
    { mtx.lock() } -> std::same_as<void>;
    { mtx.unlock() } -> std::same_as<void>;
};
}; // namespace coro::concepts
