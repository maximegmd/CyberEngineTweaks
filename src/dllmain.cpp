#include <stdafx.h>

#include "Image.h"
#include "Options.h"
#include "Utils.h"

#include "d3d12/D3D12.h"
#include "overlay/Overlay.h"
#include "scripting/LuaVM.h"
#include "window/window.h"
#include "scripting/GameHooks.h"

#pragma comment(lib, "dbghelp.lib")
#pragma comment(linker, "/DLL")

void EnableDebugPatch(Image* apImage);
void VirtualInputPatch(Image* apImage);
void SmtAmdPatch(Image* apImage);
void StartScreenPatch(Image* apImage);
void RemovePedsPatch(Image* apImage);
void OptionsPatch(Image* apImage);
void OptionsInitHook(Image* apImage);
void DisableIntroMoviesPatch(Image* apImage);
void DisableVignettePatch(Image* apImage);
void DisableBoundaryTeleportPatch(Image* apImage);

static HANDLE s_modInstanceMutex = nullptr; 

using namespace std::chrono_literals;

static void Initialize()
{
    // paths need to be initialized first
    Paths::Initialize();

    // initialize main logger as second thing, so we can log main CET messages
    set_default_logger(CreateLogger(Paths::Get().CETRoot() / "cyber_engine_tweaks.log", "main"));
    spdlog::flush_every(3s);

    Logger::Initialize();
    VKBindings::Initialize();
    Options::Initialize();

    auto& options = Options::Get();

    if (!options.ExeValid)
        return;

    if(options.GameImage.GetVersion() != Image::GetSupportedVersion())
    {
        auto [major, minor] = Image::GetSupportedVersion();
        spdlog::error("Unsupported game version! Only {}.{:02d} is supported.", major, minor);
        return;
    }

    s_modInstanceMutex = CreateMutex(NULL, TRUE, _T("Cyber Engine Tweaks Module Instance"));
    if (s_modInstanceMutex == nullptr)
        return;

    MH_Initialize();

    if (options.PatchEnableDebug)
        EnableDebugPatch(&options.GameImage);

    if(options.PatchSkipStartMenu)
        StartScreenPatch(&options.GameImage);

    if(options.PatchRemovePedestrians)
        RemovePedsPatch(&options.GameImage);

    if(options.PatchAsyncCompute || options.PatchAntialiasing)
        OptionsPatch(&options.GameImage);

    if (options.PatchDisableIntroMovies)
        DisableIntroMoviesPatch(&options.GameImage);

    if (options.PatchDisableVignette)
        DisableVignettePatch(&options.GameImage);

    if (options.PatchDisableBoundaryTeleport)
        DisableBoundaryTeleportPatch(&options.GameImage);

    OptionsInitHook(&options.GameImage);

    Window::Initialize();
    Overlay::Initialize();
    LuaVM::Initialize();
    D3D12::Initialize();

#ifndef NDEBUG
    // We only need to hook the game thread right now to do RTTI Dump, which is Debug-only
    // if we need to queue tasks to the mainthread remove the debug check
    GameMainThread::Initialize();
#endif

    MH_EnableHook(MH_ALL_HOOKS);
}

static void Shutdown()
{
    if (s_modInstanceMutex)
    {
        D3D12::Shutdown();
        LuaVM::Shutdown();
        Overlay::Shutdown();
        Window::Shutdown();

        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();

        ReleaseMutex(s_modInstanceMutex);
    }

    Logger::Shutdown();
    VKBindings::Shutdown();
    Options::Shutdown();
    //Paths::Shutdown();

    // flush main log (== default logger)
    spdlog::default_logger()->flush();
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
