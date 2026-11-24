#pragma once

#include <cstdint>
#include <limits>

namespace ShutdownCompletion
{
enum class Result { Complete, TimedOut, DeviceRemoved };

// The caller supplies the real GPU fence and a monotonic elapsed clock.
// Never classify D3D12's device-removed sentinel as successful completion.
template <typename Read, typename Elapsed, typename Pause>
Result Wait(Read read, Elapsed elapsed, Pause pause, uint64_t target, uint64_t timeoutMs)
{
    for (;;)
    {
        const auto completed = read();
        if (completed == std::numeric_limits<uint64_t>::max())
            return Result::DeviceRemoved;
        if (completed >= target)
            return Result::Complete;
        if (elapsed() >= timeoutMs)
            return Result::TimedOut;
        pause();
    }
}
}
