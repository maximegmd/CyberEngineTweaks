#include <stdafx.h>

#include "Image.h"
#include "Options.h"

#include "d3d12/D3D12.h"
#include "toolbar/Toolbar.h"
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

    s_modInstanceMutex = CreateMutex(NULL, TRUE, _T("Cyber Engine Tweaks Module Instance"));
    if (s_modInstanceMutex == nullptr)
        return;

    MH_Initialize();

    Paths::Initialize();
    Logger::Initialize();
    Options::Initialize(mod);
    auto& options = Options::Get();

    if (!options.IsCyberpunk2077())
        return;

    if(options.GameImage.GetVersion() != Image::GetSupportedVersion())
    {
        auto [major, minor] = Image::GetSupportedVersion();
        Logger::ErrorToMainFmt("Unsupported game version! Only {}.{:02d} is supported.", major, minor);
        return;
    }

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
    Toolbar::Initialize();
    LuaVM::Initialize();
    D3D12::Initialize();

    MH_EnableHook(MH_ALL_HOOKS);
}

static void Shutdown()
{
    if (s_modInstanceMutex)
    {
        D3D12::Shutdown();
        LuaVM::Shutdown();
        Toolbar::Shutdown();
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
