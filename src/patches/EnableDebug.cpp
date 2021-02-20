#include <stdafx.h>

#include "Image.h"

using ScriptExecutionPointer = uint64_t;

void HookIsFinal(void* a, ScriptExecutionPointer* apExecutionPointer, uint8_t* apReturnValue, void* d)
{
    (*apExecutionPointer)++;

    if (apReturnValue)
        *apReturnValue = 0;
}

void HookIsDebug(void* a, ScriptExecutionPointer* apExecutionPointer, uint8_t* apReturnValue, void* d)
{
    (*apExecutionPointer)++;

    if (apReturnValue)
        *apReturnValue = 1;
}

using TRegisterScriptFunction = void(void* a, uint64_t hash, uint64_t hash2, void* func);
TRegisterScriptFunction* RealRegisterScriptFunction = nullptr;

using TRegisterScriptMemberFunction = void(void* a, void* parentClass, uint64_t hash, uint64_t hash2, void* func, uint32_t flag);
TRegisterScriptMemberFunction* RealRegisterScriptMemberFunction = nullptr;

void HookRegisterScriptFunction(void* a, uint64_t hash, uint64_t hash2, void* func)
{
    // IsFinal (global)
    // if false shows debug menu option on main menu & pause menu
    if (hash == RED4ext::FNV1a("IsFinal"))
        func = &HookIsFinal;

    // AreDebugContextsEnabled (global)
    // unknown effect
    else if (hash == RED4ext::FNV1a("AreDebugContextsEnabled"))
        func = &HookIsDebug;

    RealRegisterScriptFunction(a, hash, hash2, func);
}

void HookRegisterScriptMemberFunction(void* a, void* parentClass, uint64_t hash, uint64_t hash2, void* func, uint32_t flag)
{
    // WorldMapMenuGameController::CanDebugTeleport
    // allows using world_map_menu_debug_teleport binding on map screen to teleport
    // (must be set inside r6\config\inputUserMappings.xml first)
    if (hash == RED4ext::FNV1a("CanDebugTeleport"))
        func = &HookIsDebug;

    // TargetShootComponent::IsDebugEnabled
    // unknown effect
    else if (hash == RED4ext::FNV1a("IsDebugEnabled"))
        func = &HookIsDebug;

    RealRegisterScriptMemberFunction(a, parentClass, hash, hash2, func, flag);
}

void EnableDebugPatch(const Image* apImage)
{
    uint8_t* pChecksumLocations[] = { 0, 0 };

    const mem::pattern cIsFinalPattern("48 BB 87 C9 B1 63 33 01 15 75");
    const mem::pattern cCanDebugTeleportPattern("48 BB C3 63 E3 32 7C A2 3C C1");

    const mem::default_scanner cIsFinalScanner(cIsFinalPattern);
    const mem::default_scanner cCanDebugTeleportScanner(cCanDebugTeleportPattern);

    pChecksumLocations[0] = cIsFinalScanner(apImage->TextRegion).as<uint8_t*>(); // "IsFinal", helps us find RegisterScriptFunction
    pChecksumLocations[1] = cCanDebugTeleportScanner(apImage->TextRegion).as<uint8_t*>(); // "CanDebugTeleport", to find RegisterScriptMemberFunction

    for (int i = 0; i < 2; i++)
    {
        const char* patchType = (i == 0 ? "Unlock menu patch" : "Unlock debug functions");

        uint8_t* pCallLocation = nullptr;
        if (pChecksumLocations[i])
        {
            mem::region reg(pChecksumLocations[i], 0x1000);

            if (i == 0)
            {
                const mem::pattern cPattern("48 8D 0D ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8D 0D");
                const mem::default_scanner cScanner(cPattern);
                pCallLocation = cScanner(reg).as<uint8_t*>();
            }
            else
            {
                const mem::pattern cPattern("48 8D 0D ?? ?? ?? ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 8D 0D");
                const mem::default_scanner cScanner(cPattern);
                pCallLocation = cScanner(reg).as<uint8_t*>();
            }

            if (pCallLocation)
                pCallLocation += (i == 0 ? 8 : 11);
        }

        if (pCallLocation)
        {
            DWORD oldProtect = 0;
            VirtualProtect(pCallLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
            int32_t offset = *(int32_t*)(pCallLocation)+4;
            VirtualProtect(pCallLocation, 32, oldProtect, nullptr);

            if (i == 0)
            {
                RealRegisterScriptFunction = reinterpret_cast<TRegisterScriptFunction*>(pCallLocation + offset);
                MH_CreateHook(RealRegisterScriptFunction, &HookRegisterScriptFunction, reinterpret_cast<void**>(&RealRegisterScriptFunction));
            }
            else
            {
                RealRegisterScriptMemberFunction = reinterpret_cast<TRegisterScriptMemberFunction*>(pCallLocation + offset);
                MH_CreateHook(RealRegisterScriptMemberFunction, &HookRegisterScriptMemberFunction, reinterpret_cast<void**>(&RealRegisterScriptMemberFunction));
            }

            spdlog::info("{}: success", patchType);
        }
        else
        {
            spdlog::warn("{}: failed, unknown version", patchType);
        }
    }
}
