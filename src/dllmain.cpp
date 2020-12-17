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
void UnlockMenuPatch(Image* apImage);
void VirtualInputPatch(Image* apImage);
void SmtAmdPatch(Image* apImage);
void PatchAvx(Image* apImage);
void SpectrePatch(Image* apImage);
void StringInitializerPatch(Image* apImage);
void SpinLockPatch(Image* apImage);
void StartScreenPatch(Image* apImage);
void RemovePedsPatch(Image* apImage);
void OptionsPatch(Image* apImage);

void Initialize(HMODULE mod)
{
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

    if (options.PatchUnlockMenu)
        UnlockMenuPatch(&image);

    if(options.PatchSkipStartMenu)
        StartScreenPatch(&image);

    if(options.PatchRemovePedestrians)
        RemovePedsPatch(&image);

    if(options.PatchAsyncCompute || options.PatchAntialiasing)
        OptionsPatch(&image);

    MH_EnableHook(MH_ALL_HOOKS);

    std::thread t([]()
        {
            if (kiero::init(kiero::RenderType::D3D12) != kiero::Status::Success)
            {
                spdlog::error("Kiero failed!");
            }
            else
                Overlay::Initialize();
        });
    t.detach();

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
