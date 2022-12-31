#include "stdafx.h"

void SmtAmdPatch()
{
    const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CPatches_AmdSMT);
    uint8_t* pLocation = func.GetAddr();

    if (pLocation == nullptr)
    {
        Log::Warn("AMD SMT Patch: failed");
        return;
    }

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 8, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0x74;
    VirtualProtect(pLocation, 8, oldProtect, nullptr);

    Log::Info("AMD SMT Patch: success");
}
