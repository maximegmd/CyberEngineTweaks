#include "CET.h"

#include <stdafx.h>

#include "Image.h"
#include "Options.h"
#include "scripting/GameOptions.h"

using TGameOptionInit = void*(void*);
TGameOptionInit* RealGameOptionInit = nullptr;

void* HookGameOptionInit(GameOption* apThis)
{
    auto& gameOptions = GameOptions::GetList();
    auto& options = CET::Get().GetOptions();

    if (std::find(gameOptions.begin(), gameOptions.end(), apThis) == gameOptions.end())
    {
        gameOptions.push_back(apThis);
    }

    if (options.DumpGameOptions)
        Log::Info(apThis->GetInfo());

    if (options.PatchAsyncCompute && strcmp(apThis->pCategory, "Rendering/AsyncCompute") == 0)
    {
        if (apThis->SetBool(false))
            Log::Info("Disabled hidden setting \"{}/{}\"", apThis->pCategory, apThis->pName);
        else
            Log::Warn("Failed to disable hidden setting \"{}/{}\"", apThis->pCategory, apThis->pName);
    }
    else if (options.PatchAntialiasing && (strcmp(apThis->pName, "Antialiasing") == 0 || strcmp(apThis->pName, "ScreenSpaceReflection") == 0))
    {
        if (apThis->SetBool(false))
            Log::Info("Disabled hidden setting \"{}/{}\"", apThis->pCategory, apThis->pName);
        else
            Log::Warn("Failed to disable hidden setting \"{}/{}\"", apThis->pCategory, apThis->pName);
    }

    return RealGameOptionInit(apThis);
}

void OptionsInitHook(const Image* apImage)
{
    RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CPatches_OptionsInit);
    uint8_t* pLocation = func.GetAddr();

    if (pLocation)
    {
        MH_CreateHook(pLocation, &HookGameOptionInit, reinterpret_cast<void**>(&RealGameOptionInit));
        MH_EnableHook(pLocation);

        Log::Info("Hidden options hook: success");
    }
    else
        Log::Warn("Hidden options hook: failed");
}
