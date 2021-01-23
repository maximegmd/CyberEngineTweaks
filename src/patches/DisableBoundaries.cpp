#include <stdafx.h>

#include "Image.h"

void DisableBoundaryTeleportPatch(const Image* apImage)
{
    // Disarm the WorldBoundarySystem/Tick function
    // Going out of bounds will still play the glitchy-screen effect that normally happens when game teleports you, but the actual teleport won't happen
    const mem::pattern cPattern("48 8B C4 55 53 41 54 48 8D A8 78");
    const mem::default_scanner cScanner(cPattern);
    const auto pLocation = cScanner(apImage->TextRegion).as<uint8_t*>();

    if (pLocation == nullptr)
    {
        spdlog::warn("Disable boundary teleport: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0xC3;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    spdlog::info("Disable boundary teleport: success");
}
