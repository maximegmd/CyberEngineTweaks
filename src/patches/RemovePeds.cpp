#include <stdafx.h>

#include "Image.h"

void RemovePedsPatch(const Image* apImage)
{
    const mem::pattern cPattern("3B D8 0F 4E C3 8B D8 85 DB 0F 8E");
    const mem::default_scanner scanner(cPattern);

    const auto pLocation = scanner(apImage->TextRegion).as<uint8_t*>();

    if(pLocation == nullptr)
    {
        spdlog::warn("Remove peds patch: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[9] = 0x90;
    pLocation[10] = 0xE9;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    spdlog::info("Remove peds patch: success");
}
