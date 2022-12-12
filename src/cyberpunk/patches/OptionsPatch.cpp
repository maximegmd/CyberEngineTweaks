#include <stdafx.h>

#include "EngineTweaks.h"
#include <scripting/GameOptions.h>

using TGameOptionInit = void*(void*);
TGameOptionInit* RealGameOptionInit = nullptr;

void* HookGameOptionInit(GameOption* apThis)
{
    auto& gameOptions = GameOptions::GetList();
    auto& options = EngineTweaks::Get().GetOptions();

    if (std::ranges::find(gameOptions, apThis) == gameOptions.end())
    {
        gameOptions.emplace_back(apThis);
    }

    if (options.Developer.DumpGameOptions)
        Log::Info(apThis->GetInfo());

    if (options.Patches.AsyncCompute && strcmp(apThis->pCategory, "Rendering/AsyncCompute") == 0)
    {
        if (apThis->SetBool(false))
            Log::Info("Disabled hidden setting \"{}/{}\"", apThis->pCategory, apThis->pName);
        else
            Log::Warn("Failed to disable hidden setting \"{}/{}\"", apThis->pCategory, apThis->pName);
    }
    else if (options.Patches.Antialiasing && (strcmp(apThis->pName, "Antialiasing") == 0 || strcmp(apThis->pName, "ScreenSpaceReflection") == 0))
    {
        if (apThis->SetBool(false))
            Log::Info("Disabled hidden setting \"{}/{}\"", apThis->pCategory, apThis->pName);
        else
            Log::Warn("Failed to disable hidden setting \"{}/{}\"", apThis->pCategory, apThis->pName);
    }

    return RealGameOptionInit(apThis);
}

void OptionsInitHook()
{
    const RelocPtr<uint8_t> func(Game::Addresses::CPatches_OptionsInit);
    uint8_t* pLocation = func.GetAddr();

    if (pLocation)
    {
        MH_CreateHook(pLocation, reinterpret_cast<void*>(&HookGameOptionInit), reinterpret_cast<void**>(&RealGameOptionInit));
        MH_EnableHook(pLocation);

        Log::Info("Hidden options hook: success");
    }
    else
        Log::Warn("Hidden options hook: failed");
}
