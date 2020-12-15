#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DbgHelp.h>
#include <spdlog/spdlog.h>

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

    spdlog::default_logger()->flush();
}

BOOL APIENTRY DllMain(HMODULE mod, DWORD ul_reason_for_call, LPVOID) {
    switch(ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        Initialize(mod);
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}
