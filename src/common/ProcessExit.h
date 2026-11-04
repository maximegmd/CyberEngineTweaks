#pragma once

namespace ProcessExit
{
enum class DetachAction { ProcessTermination, DynamicUnload };

constexpr DetachAction SelectAction(bool reservedIsNonNull)
{
    return reservedIsNonNull ? DetachAction::ProcessTermination : DetachAction::DynamicUnload;
}

// Only for process termination, where the OS reclaims the entire address space.
// Do not use this for runtime unload, reload, resize, or ordinary cleanup failure.
template <typename Owner> void LeaveForOperatingSystem(Owner& owner) noexcept
{
    (void)owner.release();
}
}
