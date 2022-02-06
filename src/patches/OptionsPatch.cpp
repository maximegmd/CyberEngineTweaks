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
    const mem::pattern cPattern("48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 40 48 8B F1 48 8D 4C 24 20 E8");
    const mem::default_scanner cScanner(cPattern);
    auto GameOptionInit = cScanner(apImage->TextRegion).as<uint8_t*>();

    if (GameOptionInit)
    {
        MH_CreateHook(GameOptionInit, &HookGameOptionInit, reinterpret_cast<void**>(&RealGameOptionInit));
        MH_EnableHook(GameOptionInit);

        Log::Info("Hidden options hook: success");
    }
    else
        Log::Warn("Hidden options hook: failed");
}
