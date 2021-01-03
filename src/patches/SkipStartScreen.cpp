#include <stdafx.h>

#include "Image.h"
#include "Pattern.h"

void StartScreenPatch(Image* apImage)
{
    auto pLocation = FindSignature(apImage->pTextStart, apImage->pTextEnd, { 0x48, 0xBB , 0xE6 , 0xF8 , 0xA5, 0xA3, 0x36, 0x56, 0x4E, 0xA7, 0xC6 , 0x85, 0xB0, 0xCC, 0xCC, 0xCC, 0x01 });
    if(pLocation == nullptr)
    {
        spdlog::info("Start screen patch: failed, could not be found");
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
