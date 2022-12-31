#include "stdafx.h"

void DisableBoundaryTeleportPatch()
{
    // Disarm the WorldBoundarySystem/Tick function
    // Going out of bounds will still play the glitchy-screen effect that normally happens when game teleports you, but
    // the actual teleport won't happen
    const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CPatches_BoundaryTeleport);
    const auto pLocation = func.GetAddr();

    if (pLocation == nullptr)
    {
        Log::Warn("Disable boundary teleport: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0xC3;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    Log::Info("Disable boundary teleport: success");
}
