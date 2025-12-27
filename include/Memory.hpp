#pragma once

#include <memory_resource>

namespace ARQ {
namespace Memory {
inline std::pmr::monotonic_buffer_resource sim_arena;

inline std::pmr::unsynchronized_pool_resource packet_pool{&sim_arena};
inline std::pmr::memory_resource *packets() noexcept { return &packet_pool; }

} // namespace Memory
} // namespace ARQ
