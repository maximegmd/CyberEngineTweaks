#include "stdafx.h"

void RemovePedsPatch()
{
    /* RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CPatches_);
    uint8_t* pLocation = func.GetAddr();

    if(pLocation == nullptr)
    {
        Log::Warn("Remove peds patch: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[16] = 0x90;
    pLocation[17] = 0xE9;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);*/

    Log::Info("Remove peds patch: disabled for now");
}
