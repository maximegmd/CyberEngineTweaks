#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DbgHelp.h>
#include <spdlog/spdlog.h>
#include <kiero/kiero.h>
#include <overlay/Overlay.h>
#include <MinHook.h>
#include <thread>

#include "Image.h"
#include "Options.h"

#pragma comment( lib, "dbghelp.lib" )
#pragma comment(linker, "/DLL")

void PoolPatch(Image* apImage);
void EnableDebugPatch(Image* apImage);
void VirtualInputPatch(Image* apImage);
void SmtAmdPatch(Image* apImage);
void PatchAvx(Image* apImage);
void SpectrePatch(Image* apImage);
void StringInitializerPatch(Image* apImage);
void SpinLockPatch(Image* apImage);
void StartScreenPatch(Image* apImage);
void RemovePedsPatch(Image* apImage);
void OptionsPatch(Image* apImage);
void OptionsInitPatch(Image* apImage);
void DisableIntroMoviesPatch(Image* apImage);
void DisableVignettePatch(Image* apImage);
void DisableBoundaryTeleportPatch(Image* apImage);

void Initialize(HMODULE mod)
{
    MH_Initialize();

    Options::Initialize(mod);
    const auto& options = Options::Get();

    if (!options.IsCyberpunk2077())
        return;

    Image image;

    if(options.PatchSMT)
        SmtAmdPatch(&image);

    if(options.PatchSpectre)
        SpectrePatch(&image);

    if (options.PatchAVX)
        PatchAvx(&image);

    if(options.PatchMemoryPool)
        PoolPatch(&image);

    if (options.PatchVirtualInput)
        VirtualInputPatch(&image);

    if (options.PatchEnableDebug)
        EnableDebugPatch(&image);

    if(options.PatchSkipStartMenu)
        StartScreenPatch(&image);

    if(options.PatchRemovePedestrians)
        RemovePedsPatch(&image);

    if(options.PatchAsyncCompute || options.PatchAntialiasing)
        OptionsPatch(&image);

    if (options.PatchDisableIntroMovies)
        DisableIntroMoviesPatch(&image);

    if (options.PatchDisableVignette)
        DisableVignettePatch(&image);

    if (options.PatchDisableBoundaryTeleport)
        DisableBoundaryTeleportPatch(&image);

    if (options.DumpGameOptions)
        OptionsInitPatch(&image);

    if(options.Console)
        Overlay::Initialize(&image);

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
    kiero::shutdown();
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
