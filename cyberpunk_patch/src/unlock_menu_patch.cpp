#include "Image.h"
#include <spdlog/spdlog.h>
#include <mhook-lib/mhook.h>


void HookIsFinal(void* a, uint64_t* b, char* c)
{
    (*b)++;
    if (c)
        *c = 0;
}

using TRegisterScriptFunction = void(void* a, uint64_t hash, uint64_t hash2, void* func);
TRegisterScriptFunction* RealRegisterScriptFunction = nullptr;

void HookRegisterScriptFunction(void* a, uint64_t hash, uint64_t hash2, void* func)
{
    if (hash == 0x7515013363B1C987ull)
        func = &HookIsFinal;

    RealRegisterScriptFunction(a, hash, hash2, func);
}

void UnlockMenuPatch(Image* apImage)
{
    if (apImage->version == Image::MakeVersion(1, 4))
    {
        RealRegisterScriptFunction = reinterpret_cast<TRegisterScriptFunction*>(apImage->base_address + 0x224C70);
    }
    else
    {
        spdlog::warn("\tUnlock menu patch: failed, unknown version");
        return;
    }

    Mhook_SetHook(reinterpret_cast<void**>(&RealRegisterScriptFunction), &HookRegisterScriptFunction);
    spdlog::info("\tUnlock menu patch: success");
}
