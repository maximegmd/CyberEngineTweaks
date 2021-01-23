#include <stdafx.h>

#include "Image.h"

void DisableVignettePatch(const Image* apImage)
{
    const mem::pattern cPattern("48 8B 41 30 48 83 78 68 00 74");
    const mem::default_scanner scanner(cPattern);

    const auto pLocation = scanner(apImage->TextRegion).as<uint8_t*>();

    if (pLocation == nullptr)
    {
        spdlog::warn("Disable vignette patch: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0x32;
    pLocation[1] = 0xC0;
    pLocation[2] = 0xC3;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    spdlog::info("Disable vignette patch: success");
}
