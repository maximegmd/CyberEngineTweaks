#include <stdafx.h>

#include <kiero/kiero.h>
#include <overlay/Overlay.h>

#include "Image.h"
#include "Options.h"

#pragma comment( lib, "dbghelp.lib" )
#pragma comment(linker, "/DLL")

void EnableDebugPatch(Image* apImage);
void VirtualInputPatch(Image* apImage);
void SmtAmdPatch(Image* apImage);
void PatchAvx(Image* apImage);
void StartScreenPatch(Image* apImage);
void RemovePedsPatch(Image* apImage);
void OptionsPatch(Image* apImage);
void OptionsInitHook(Image* apImage);
void DisableIntroMoviesPatch(Image* apImage);
void DisableVignettePatch(Image* apImage);
void DisableBoundaryTeleportPatch(Image* apImage);

void Initialize(HMODULE mod)
{
    MH_Initialize();

    Options::Initialize(mod);
    auto& options = Options::Get();

    if (!options.IsCyberpunk2077())
        return;

    if(options.PatchSMT)
        SmtAmdPatch(&options.GameImage);

    if (options.PatchAVX && options.GameImage.version <= Image::MakeVersion(1, 4))
        PatchAvx(&options.GameImage);

    if (options.PatchVirtualInput)
        VirtualInputPatch(&options.GameImage);

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

    if(options.Console)
        Overlay::Initialize(&options.GameImage);

    MH_EnableHook(MH_ALL_HOOKS);

    if (options.Console)
    {
        std::thread t([]()
            {
                if (kiero::init(kiero::RenderType::D3D12) != kiero::Status::Success)
                {
                    spdlog::error("Kiero failed!");
                }
                else
                    Overlay::Get().Hook();
            });
        t.detach();
    }

    spdlog::default_logger()->flush();
}

void Shutdown()
{
    if(Options::Get().Console)
        Overlay::Shutdown();

    kiero::shutdown();

    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD ul_reason_for_call, LPVOID) {
    switch(ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        Initialize(mod);
        break;

    case DLL_PROCESS_DETACH:
        Shutdown();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}
