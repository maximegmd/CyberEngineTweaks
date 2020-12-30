#include <stdafx.h>

#include "Image.h"
#include "Pattern.h"

void SaveLockPatch(Image* apImage)
{
    auto pLocation = FindSignature(apImage->pTextStart, apImage->pTextEnd, { 0x0F, 0x84, 0xCA, 0x00, 0x00, 0x00, 0x4D, 0x8B, 0x07 });
    if (pLocation == nullptr)
    {
        spdlog::info("\tRemove save lock patch: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[1] = 0xE9;
    pLocation[2] = 0xCB;
    pLocation[3] = 0x00;
    pLocation[4] = 0x00;
    pLocation[5] = 0x00;
    pLocation[6] = 0x90;

    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    spdlog::info("\tRemove save lock patch: success");
}
