#include <stdafx.h>

#include "Image.h"

void SmtAmdPatch(const Image* apImage)
{
    const mem::pattern cPattern("75 2D 33 C9 B8 01 00 00 00 0F A2 8B C8 C1 F9 08");
    const mem::default_scanner cScanner(cPattern);
    auto pLocation = cScanner(apImage->TextRegion).as<uint8_t*>();

    if (pLocation == nullptr)
    {
        spdlog::warn("AMD SMT Patch: failed");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 8, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0x74;
    VirtualProtect(pLocation, 8, oldProtect, nullptr);

    spdlog::info("AMD SMT Patch: success");
}
