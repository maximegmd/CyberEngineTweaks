#include "CET.h"

#include <stdafx.h>

#include "Image.h"
#include "Options.h"

#include "scripting/GameHooks.h"

#pragma comment(lib, "dbghelp.lib")
#pragma comment(linker, "/DLL")

void EnableDebugPatch(const Image* apImage);
void StartScreenPatch(const Image* apImage);
void RemovePedsPatch(const Image* apImage);
void OptionsPatch(const Image* apImage);
void OptionsInitHook(const Image* apImage);
void DisableIntroMoviesPatch(const Image* apImage);
void DisableVignettePatch(const Image* apImage);
void DisableBoundaryTeleportPatch(const Image* apImage);

static HANDLE s_modInstanceMutex = nullptr;

using namespace std::chrono_literals;

static void Initialize()
{
    try
    {
        MH_Initialize();

        CET::Initialize();

        const auto& options = CET::Get().GetOptions();

        // check if we are hooked to valid process
        if (!options.ExeValid)
            return;

        if (options.GameImage.GetVersion() != Image::GetSupportedVersion())
        {
            auto [major, minor] = Image::GetSupportedVersion();
            spdlog::error("Unsupported game version! Only {}.{:02d} is supported.", major, minor);
            return;
        }

        // single instance check
        s_modInstanceMutex = CreateMutex(NULL, TRUE, _T("Cyber Engine Tweaks Module Instance"));
        if (s_modInstanceMutex == nullptr)
            return;

        // initialize patches
        if (options.PatchEnableDebug)
            EnableDebugPatch(&options.GameImage);

        if (options.PatchSkipStartMenu)
            StartScreenPatch(&options.GameImage);

        if (options.PatchRemovePedestrians)
            RemovePedsPatch(&options.GameImage);

        if (options.PatchAsyncCompute || options.PatchAntialiasing)
            OptionsPatch(&options.GameImage);

        if (options.PatchDisableIntroMovies)
            DisableIntroMoviesPatch(&options.GameImage);

        if (options.PatchDisableVignette)
            DisableVignettePatch(&options.GameImage);

        if (options.PatchDisableBoundaryTeleport)
            DisableBoundaryTeleportPatch(&options.GameImage);

        OptionsInitHook(&options.GameImage);


#ifndef NDEBUG
        // We only need to hook the game thread right now to do RTTI Dump, which is Debug-only
        // if we need to queue tasks to the mainthread remove the debug check
        GameMainThread::Initialize();
#endif

        MH_EnableHook(MH_ALL_HOOKS);
    }
    catch(...)
    {}
}

static void Shutdown()
{
    if (s_modInstanceMutex)
    {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();

        CET::Shutdown();

        ReleaseMutex(s_modInstanceMutex);
    }

    // flush main log (== default logger)
    spdlog::default_logger()->flush();
    spdlog::get("scripting")->flush();
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
