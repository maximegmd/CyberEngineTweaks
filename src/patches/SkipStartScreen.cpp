#include <stdafx.h>

#include "Image.h"

void StartScreenPatch(const Image* apImage)
{
    const mem::pattern cPattern("48 BB E6 F8 A5 A3 36 56 4E A7 C6 85 B0 ?? ?? ?? 01");
    const mem::default_scanner cScanner(cPattern);
    auto pLocation = cScanner(apImage->TextRegion).as<uint8_t*>();

    if(pLocation == nullptr)
    {
        spdlog::warn("Start screen patch: failed, could not be found");
        return;
    }

    pLocation -= 9;

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0x90;
    pLocation[1] = 0x90;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    spdlog::info("Start screen patch: success");
}
