#include <stdafx.h>

#include "Image.h"

void StartScreenPatch(const Image* apImage)
{
    RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CPatches_SkipStartScreen);
    uint8_t* pLocation = func.GetAddr();

    if(pLocation == nullptr)
    {
        Log::Warn("Start screen patch: failed, could not be found");
        return;
    }

    pLocation -= 9;

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0x90;
    pLocation[1] = 0x90;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    Log::Info("Start screen patch: success");
}
