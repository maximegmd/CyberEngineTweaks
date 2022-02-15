#include <stdafx.h>

#include "Image.h"

void RemovePedsPatch(const Image* apImage)
{
    RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CPatches_RemovePedestrians);
    uint8_t* pLocation = func.GetAddr();

    if(pLocation == nullptr)
    {
        Log::Warn("Remove peds patch: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[9] = 0x90;
    pLocation[10] = 0xE9;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    Log::Info("Remove peds patch: success");
}
