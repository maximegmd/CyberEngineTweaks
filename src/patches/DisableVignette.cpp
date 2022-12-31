#include "stdafx.h"

void DisableVignettePatch()
{
    const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CPatches_Vignette);
    const auto pLocation = func.GetAddr();

    if (pLocation == nullptr)
    {
        Log::Warn("Disable vignette patch: failed, could not be found");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0x32;
    pLocation[1] = 0xC0;
    pLocation[2] = 0xC3;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    Log::Info("Disable vignette patch: success");
}
