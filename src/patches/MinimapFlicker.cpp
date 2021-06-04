#include <stdafx.h>

#include "Image.h"

void MinimapFlickerPatch(const Image* apImage)
{
    const mem::pattern cPattern("F3 44 0F 10 10 F3 45 0F 5F D7 F3 45 0F 5D D5 E8 ? ? ? ? 80 3D ? ? ? ? 00 44 0F 28 C8 F3 45 0F 5F CF F3 45 0F 5D CD");
    const mem::default_scanner cScanner(cPattern);
    auto pLocation = cScanner(apImage->TextRegion).as<uint8_t*>();

    if (pLocation == nullptr)
    {
        spdlog::warn("Minimap Flicker Patch: failed");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[26] = 0x01;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    spdlog::info("Minimap Flicker Patch: success");
}
