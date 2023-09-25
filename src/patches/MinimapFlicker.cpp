#include <stdafx.h>

/**

Does not seem to be needed since 2.0, keeping in case we need it again

void MinimapFlickerPatch()
{
    const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CPatches_MinimapFlicker);
    uint8_t* pLocation = func.GetAddr();

    if (pLocation == nullptr)
    {
        Log::Warn("Minimap Flicker Patch: failed");
        return;
    }

    pLocation += 0xEC;

    DWORD oldProtect = 0;
    VirtualProtect(pLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
    pLocation[0] = 0x01;
    VirtualProtect(pLocation, 32, oldProtect, nullptr);

    Log::Info("Minimap Flicker Patch: success");
}
*/
