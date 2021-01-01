#include <stdafx.h>

#include "Image.h"
#include "Pattern.h"

void RemovePedsPatch(Image* apImage)
{
    auto pLocation = FindSignature(apImage->pTextStart, apImage->pTextEnd, { 0x3B, 0xD8, 0x0F, 0x4E, 0xC3, 0x8B, 0xD8, 0x85, 0xDB, 0x0F, 0x8E });
    if(pLocation == nullptr)
    {
        spdlog::info("Remove peds patch: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[9] = 0x90;
    pLocation[10] = 0xE9;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    spdlog::info("Remove peds patch: success");
}
