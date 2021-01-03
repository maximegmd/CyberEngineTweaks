#include <stdafx.h>

#include "Image.h"
#include "Pattern.h"

void DisableBoundaryTeleportPatch(Image* apImage)
{
    // Disarm the WorldBoundarySystem/Tick function
    // Going out of bounds will still play the glitchy-screen effect that normally happens when game teleports you, but the actual teleport won't happen
    auto* pLocation = FindSignature(apImage->pTextStart, apImage->pTextEnd, {
        0x48, 0x8B, 0xC4, 0x55, 0x53, 0x41, 0x54, 0x48, 0x8D, 0xA8, 0x78
        });

    if (pLocation == nullptr)
    {
        spdlog::info("Disable boundary teleport: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0xC3;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    spdlog::info("Disable boundary teleport: success");
}
