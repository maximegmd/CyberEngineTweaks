#include <stdafx.h>

#include "CET.h"

#include "Options.h"

#include "scripting/GameHooks.h"

void StartScreenPatch();
void OptionsInitHook();
void DisableIntroMoviesPatch();
void DisableVignettePatch();
void DisableBoundaryTeleportPatch();

static HANDLE s_modInstanceMutex = nullptr;

using namespace std::chrono_literals;

static void Initialize()
{
    try
    {
        MH_Initialize();

        CET::Initialize();

        const auto& options = CET::Get().GetOptions();

        // single instance check
        s_modInstanceMutex = CreateMutex(nullptr, TRUE, TEXT("Cyber Engine Tweaks Module Instance"));
        if (s_modInstanceMutex == nullptr)
            return;

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
    }
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

BOOL APIENTRY DllMain(HMODULE mod, DWORD ul_reason_for_call, LPVOID)
{
    DisableThreadLibraryCalls(mod);

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: Initialize(); break;
    case DLL_PROCESS_DETACH: Shutdown(); break;
    default: break;
    }

    return TRUE;
}
