#include "stdafx.h"

void StartScreenPatch()
{
    const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CPatches_SkipStartScreen);
    uint8_t* pLocation = func.GetAddr();

    if (pLocation == nullptr)
    {
        Log::Warn("Start screen patch: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0xE9;
    pLocation[1] = 0x08;
    pLocation[2] = 0x01;
    pLocation[3] = 0x00;
    pLocation[4] = 0x00;
    pLocation[5] = 0x00;
    pLocation[6] = 0x00;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    Log::Info("Start screen patch: success");
}
