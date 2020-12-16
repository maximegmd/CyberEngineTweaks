#include "Image.h"
#include <spdlog/spdlog.h>
#include <mhook-lib/mhook.h>
#include "REDString.h"
#include "Pattern.h"

using ScriptExecutionPointer = uint64_t;

void HookIsFinal(void* a, ScriptExecutionPointer* apExecutionPointer, uint8_t* apReturnValue, void* d)
{
    (*apExecutionPointer)++;

    if (apReturnValue)
        *apReturnValue = 0;
}

using TRegisterScriptFunction = void(void* a, uint64_t hash, uint64_t hash2, void* func);
TRegisterScriptFunction* RealRegisterScriptFunction = nullptr;

void HookRegisterScriptFunction(void* a, uint64_t hash, uint64_t hash2, void* func)
{
    if (hash == REDString::Hash("IsFinal"))
        func = &HookIsFinal;

    RealRegisterScriptFunction(a, hash, hash2, func);
}

void UnlockMenuPatch(Image* apImage)
{
    const auto pChecksumLocation = FindSignature(apImage->pTextStart, apImage->pTextEnd, { 0x48, 0xBB, 0x87, 0xC9, 0xB1, 0x63, 0x33, 0x01, 0x15, 0x75 });
    uint8_t* pCallLocation = nullptr;
    if(pChecksumLocation)
    {
        pCallLocation = FindSignature(pChecksumLocation, pChecksumLocation + 512, { 0x48, 0x8D, 0x0D, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8, 0xCC, 0xCC, 0xCC, 0xCC, 0x48, 0x8D, 0x0D });
        if(pCallLocation)
        {
            pCallLocation += 8;
        }
    }

    if (pCallLocation)
    {
        DWORD oldProtect = 0;
        VirtualProtect(pCallLocation, 32, PAGE_EXECUTE_WRITECOPY, &oldProtect);
        uint32_t offset = *(uint32_t*)(pCallLocation) + 4;       
        VirtualProtect(pCallLocation, 32, oldProtect, nullptr);

        RealRegisterScriptFunction =  reinterpret_cast<TRegisterScriptFunction*>(pCallLocation + offset);
    }
    else
    {
        spdlog::warn("\tUnlock menu patch: failed, unknown version");
        return;
    }

    Mhook_SetHook(reinterpret_cast<void**>(&RealRegisterScriptFunction), &HookRegisterScriptFunction);
    spdlog::info("\tUnlock menu patch: success");
}
