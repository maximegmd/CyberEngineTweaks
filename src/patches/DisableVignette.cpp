#include <stdafx.h>

#include "Image.h"
#include "Pattern.h"

void DisableVignettePatch(Image* apImage)
{
    auto* pLocation = FindSignature(apImage->pTextStart, apImage->pTextEnd, {
        0x48, 0x8B, 0x41, 0x30, 0x48, 0x83, 0x78, 0x68, 0x00, 0x74
        });

    if (pLocation == nullptr)
    {
        spdlog::info("Disable vignette patch: failed, could not be found");
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
