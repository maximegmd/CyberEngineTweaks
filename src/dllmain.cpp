#include <stdafx.h>

#include "Image.h"
#include "Options.h"

#include "d3d12/D3D12.h"
#include "overlay/Overlay.h"
#include "scripting/LuaVM.h"
#include "window/window.h"

#pragma comment( lib, "dbghelp.lib" )
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

static void Initialize(HMODULE mod)
{
    Paths::Initialize();
    Logger::Initialize();
    VKBindings::Initialize();
    Options::Initialize();

    if (!Options::Initialized || !Options::ExeValid)
        return;

    if(Options::GameImage.GetVersion() != Image::GetSupportedVersion())
    {
        auto [major, minor] = Image::GetSupportedVersion();
        Logger::ErrorToMainFmt("Unsupported game version! Only {}.{:02d} is supported.", major, minor);
        return;
    }

    s_modInstanceMutex = CreateMutex(NULL, TRUE, _T("Cyber Engine Tweaks Module Instance"));
    if (s_modInstanceMutex == nullptr)
        return;

    MH_Initialize();

    if (Options::PatchEnableDebug)
        EnableDebugPatch(&Options::GameImage);

    if(Options::PatchSkipStartMenu)
        StartScreenPatch(&Options::GameImage);

    if(Options::PatchRemovePedestrians)
        RemovePedsPatch(&Options::GameImage);

    if(Options::PatchAsyncCompute || Options::PatchAntialiasing)
        OptionsPatch(&Options::GameImage);

    if (Options::PatchDisableIntroMovies)
        DisableIntroMoviesPatch(&Options::GameImage);

    if (Options::PatchDisableVignette)
        DisableVignettePatch(&Options::GameImage);

    if (Options::PatchDisableBoundaryTeleport)
        DisableBoundaryTeleportPatch(&Options::GameImage);

    OptionsInitHook(&Options::GameImage);

    Window::Initialize();
    Overlay::Initialize();
    LuaVM::Initialize();
    D3D12::Initialize();

    MH_EnableHook(MH_ALL_HOOKS);
}

static void Shutdown()
{
    if (s_modInstanceMutex)
    {
        // shutduwn these two first always, so we know for sure they got saved properly! (they are not invalidated by this)
        //Options::Shutdown();
        //VKBindings::Shutdown();

        D3D12::Shutdown();
        LuaVM::Shutdown();
        Overlay::Shutdown();
        Window::Shutdown();
        Logger::Shutdown();

        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();

        ReleaseMutex(s_modInstanceMutex);
    }
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD ul_reason_for_call, LPVOID) 
{
    DisableThreadLibraryCalls(mod);

    switch(ul_reason_for_call) 
    {
        case DLL_PROCESS_ATTACH:
            Initialize(mod);
            break;
        case DLL_PROCESS_DETACH:
            Shutdown();
            break;
        default:
            break;
    }

    return TRUE;
}
