#include <stdafx.h>

#include "CET.h"

#include "Options.h"
#include "RED4ext/Api/EMainReason.hpp"
#include "RED4ext/Api/PluginHandle.hpp"
#include "RED4ext/Api/Runtime.hpp"
#include "RED4ext/Api/Sdk.hpp"
#include "RED4ext/Api/Version.hpp"

#include "scripting/GameHooks.h"

void StartScreenPatch();
void OptionsInitHook();
void DisableIntroMoviesPatch();
void DisableVignettePatch();
void DisableBoundaryTeleportPatch();

static HANDLE s_modInstanceMutex = nullptr;

using namespace std::chrono_literals;

static bool Initialize()
{
    try
    {
        MH_Initialize();

        CET::Initialize();

        const auto& options = CET::Get().GetOptions();

        // single instance check
        s_modInstanceMutex = CreateMutex(nullptr, TRUE, TEXT("Cyber Engine Tweaks Module Instance"));
        if (s_modInstanceMutex == nullptr)
            return false;

        // initialize patches

        //if (options.Patches.SkipStartMenu)
        //    StartScreenPatch();

        if (options.Patches.DisableIntroMovies)
            DisableIntroMoviesPatch();

        if (options.Patches.DisableVignette)
            DisableVignettePatch();

        if (options.Patches.DisableBoundaryTeleport)
            DisableBoundaryTeleportPatch();


        OptionsInitHook();

        MH_EnableHook(nullptr);
    }
    catch (...)
    {
        return false;
    }

    return true;
}

static void Shutdown()
{
    bool inGameProcess = false;

    if (s_modInstanceMutex)
    {
        inGameProcess = CET::Get().GetOptions().ExeValid;

        MH_DisableHook(nullptr);
        MH_Uninitialize();

        CET::Shutdown();

        ReleaseMutex(s_modInstanceMutex);
    }

    if (inGameProcess)
    {
        // flush main log (== default logger)
        spdlog::default_logger()->flush();
        spdlog::get("scripting")->flush();
    }
}

RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::PluginHandle aHandle, RED4ext::EMainReason aReason, const RED4ext::Sdk* aSdk)
{
    RED4EXT_UNUSED_PARAMETER(aHandle);
    RED4EXT_UNUSED_PARAMETER(aSdk);

    switch (aReason)
    {
    case RED4ext::EMainReason::Load:
    {
        return Initialize();
        break;
    }
    case RED4ext::EMainReason::Unload:
    {
        Shutdown();
        break;
    }
    }

    return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::PluginInfo* aInfo)
{
    aInfo->name = L"Cyber Engine Tweaks";
    aInfo->author = L"Yamashi and Friends";

    std::istringstream oss(CET_BUILD_COMMIT);

    char buffer;
    uint32_t major, minor, patch;
    oss >> buffer >> major >> buffer >> minor >> buffer >> patch;

    aInfo->version = RED4EXT_SEMVER(major, minor, patch);
    aInfo->runtime = RED4EXT_RUNTIME_INDEPENDENT;
    aInfo->sdk = RED4EXT_SDK_LATEST;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_LATEST;
}
