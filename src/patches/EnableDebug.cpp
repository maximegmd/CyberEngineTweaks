#include "stdafx.h"

using ScriptExecutionPointer = uint64_t;

void HookIsFinal(void*, ScriptExecutionPointer* apExecutionPointer, uint8_t* apReturnValue, void*)
{
    (*apExecutionPointer)++;

    if (apReturnValue)
        *apReturnValue = 0;
}

void HookIsDebug(void*, ScriptExecutionPointer* apExecutionPointer, uint8_t* apReturnValue, void*)
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
    if (hash == RED4ext::FNV1a64("IsFinal"))
        func = reinterpret_cast<void*>(&HookIsFinal);

    // AreDebugContextsEnabled (global)
    // unknown effect
    else if (hash == RED4ext::FNV1a64("AreDebugContextsEnabled"))
        func = reinterpret_cast<void*>(&HookIsDebug);

    RealRegisterScriptFunction(a, hash, hash2, func);
}

void HookRegisterScriptMemberFunction(void* a, void* parentClass, uint64_t hash, uint64_t hash2, void* func, uint32_t flag)
{
    // WorldMapMenuGameController::CanDebugTeleport
    // allows using world_map_menu_debug_teleport binding on map screen to teleport
    // (must be set inside r6\config\inputUserMappings.xml first)
    if (hash == RED4ext::FNV1a64("CanDebugTeleport"))
        func = reinterpret_cast<void*>(&HookIsDebug);

    // TargetShootComponent::IsDebugEnabled
    // unknown effect
    else if (hash == RED4ext::FNV1a64("IsDebugEnabled"))
        func = reinterpret_cast<void*>(&HookIsDebug);

    RealRegisterScriptMemberFunction(a, parentClass, hash, hash2, func, flag);
}

void EnableDebugPatch()
{
    const RED4ext::RelocPtr<uint8_t> registerFunction(RED4ext::Addresses::CBaseFunction_Register);
    const RED4ext::RelocPtr<uint8_t> registerMemberFunction(CyberEngineTweaks::Addresses::CScript_RegisterMemberFunction);

    RealRegisterScriptFunction = reinterpret_cast<TRegisterScriptFunction*>(registerFunction.GetAddr());
    MH_CreateHook(reinterpret_cast<void*>(RealRegisterScriptFunction), reinterpret_cast<void*>(&HookRegisterScriptFunction), reinterpret_cast<void**>(&RealRegisterScriptFunction));

    RealRegisterScriptMemberFunction = reinterpret_cast<TRegisterScriptMemberFunction*>(registerMemberFunction.GetAddr());
    MH_CreateHook(
        reinterpret_cast<void*>(RealRegisterScriptMemberFunction), reinterpret_cast<void*>(&HookRegisterScriptMemberFunction),
        reinterpret_cast<void**>(&RealRegisterScriptMemberFunction));
}
