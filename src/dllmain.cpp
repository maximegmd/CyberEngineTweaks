#include <stdafx.h>

#include "EngineTweaks.h"

#include "Options.h"

#include "scripting/GameHooks.h"

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

        EngineTweaks::Initialize();

        const auto& options = EngineTweaks::Get().GetOptions();

        // single instance check
        s_modInstanceMutex = CreateMutex(nullptr, TRUE, _T("Cyber Engine Tweaks Module Instance"));
        if (s_modInstanceMutex == nullptr)
            return;

#if GAME_CYBERPUNK
        // initialize patches
        if (options.Developer.EnableDebug)
            EnableDebugPatch();

        if (options.Patches.SkipStartMenu)
            StartScreenPatch();

        if (options.Patches.RemovePedestrians)
            RemovePedsPatch();

        if (options.Patches.DisableIntroMovies)
            DisableIntroMoviesPatch();

        if (options.Patches.DisableVignette)
            DisableVignettePatch();

        if (options.Patches.DisableBoundaryTeleport)
            DisableBoundaryTeleportPatch();

        if (options.Patches.AmdSmt)
            SmtAmdPatch();

        if (options.Patches.MinimapFlicker)
            MinimapFlickerPatch();

        OptionsInitHook();
#endif

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
        inGameProcess = EngineTweaks::Get().GetOptions().ExeValid;

        MH_DisableHook(nullptr);
        MH_Uninitialize();

        EngineTweaks::Shutdown();

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
#if GAME_CYBERPUNK
    DisableThreadLibraryCalls(mod);

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: Initialize(); break;
    case DLL_PROCESS_DETACH: Shutdown(); break;
    default: break;
    }
#endif

    return TRUE;
}

#if GAME_WITCHER
enum class EMainReason : std::uint8_t
{
    Load = 0,
    Unload
};

extern "C" __declspec(dllexport) bool Main(HMODULE aHandle, EMainReason aReason)
{
    switch (aReason)
    {
    case EMainReason::Load:
    {
        Initialize();
        break;
    }
    case EMainReason::Unload:
    {
        Shutdown();
        break;
    }
    }

    return true;
}

extern "C" __declspec(dllexport) const char* Supports()
{
    return "4.0.0.65171";
}
#endif
