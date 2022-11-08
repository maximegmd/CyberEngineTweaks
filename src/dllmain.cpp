#include <stdafx.h>

#include "CET.h"

#include "Options.h"

#ifdef CET_DEBUG
#include "scripting/GameHooks.h"
#endif

void EnableDebugPatch();
void StartScreenPatch();
void RemovePedsPatch();
void OptionsInitHook();
void DisableIntroMoviesPatch();
void DisableVignettePatch();
void DisableBoundaryTeleportPatch();
void SmtAmdPatch();
void MinimapFlickerPatch();

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
        s_modInstanceMutex = CreateMutex(nullptr, TRUE, _T("Cyber Engine Tweaks Module Instance"));
        if (s_modInstanceMutex == nullptr)
            return;

        // initialize patches
        if (options.PatchEnableDebug)
            EnableDebugPatch();

        if (options.PatchSkipStartMenu)
            StartScreenPatch();

        if (options.PatchRemovePedestrians)
            RemovePedsPatch();

        if (options.PatchDisableIntroMovies)
            DisableIntroMoviesPatch();

        if (options.PatchDisableVignette)
            DisableVignettePatch();

        if (options.PatchDisableBoundaryTeleport)
            DisableBoundaryTeleportPatch();

        if (options.PatchAmdSmt)
            SmtAmdPatch();

        if (options.PatchMinimapFlicker)
            MinimapFlickerPatch();

        OptionsInitHook();


#ifdef CET_DEBUG
        // We only need to hook the game thread right now to do RTTI Dump, which is Debug-only
        // if we need to queue tasks to the mainthread remove the debug check
        GameMainThread::Initialize();
#endif

        MH_EnableHook(nullptr);
    }
    catch(...)
    {}
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

    switch(ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            Initialize();
            break;
        case DLL_PROCESS_DETACH:
            Shutdown();
            break;
        default:
            break;
    }

    return TRUE;
}
