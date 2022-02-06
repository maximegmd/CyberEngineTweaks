#include <stdafx.h>

#include "Image.h"

void MinimapFlickerPatch(const Image* apImage)
{
    const mem::pattern cPattern("83 79 2C 00 48 8B F2 4C");
    const mem::default_scanner cScanner(cPattern);
    auto pLocation = cScanner(apImage->TextRegion).as<uint8_t*>();

    if (pLocation == nullptr)
    {
        Log::Warn("Minimap Flicker Patch: failed");
        return;
    }

    pLocation += 0xEB;

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0x01;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    Log::Info("Minimap Flicker Patch: success");
}
