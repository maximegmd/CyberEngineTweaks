#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <DbgHelp.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
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
void StringInitializerPatch(Image* apImage);
void SpinLockPatch(Image* apImage);
void StartScreenPatch(Image* apImage);
void RemovePedsPatch(Image* apImage);
void OptionsPatch(Image* apImage);
void OptionsInitHook(Image* apImage);
void DisableIntroMoviesPatch(Image* apImage);
void DisableVignettePatch(Image* apImage);
void DisableBoundaryTeleportPatch(Image* apImage);


void Initialize_logger_path(HMODULE aModule)
{
    char parentPath[2048 + 1] = { 0 };
    GetModuleFileNameA(GetModuleHandleA(nullptr), parentPath, std::size(parentPath) - 1);
    Options::ExeName = std::filesystem::path(parentPath).filename().string();
 
    char path[2048 + 1] = { 0 };
    GetModuleFileNameA(aModule, path, std::size(path) - 1);
 
    Options::Path = path;
    Options::Path = Options::Path.parent_path().parent_path();

    Options::Path /= "plugins";

    Options::Path /= "cyber_engine_tweaks/";
  
    std::error_code ec;
    create_directories(Options::Path, ec);
}

unsigned long __stdcall  Initialize(HMODULE mod)
{
    Initialize_logger_path(mod);

    auto rotatingLogger = std::make_shared<spdlog::sinks::rotating_file_sink_mt>((Options::Path / "cyber_engine_tweaks.log").string(), 1048576 * 5, 3);
    auto logger = std::make_shared<spdlog::logger>("", spdlog::sinks_init_list{ rotatingLogger });
    logger->flush_on(spdlog::level::debug);
    set_default_logger(logger);
    spdlog::info("logger_init_Ok");

    MH_Initialize();
    spdlog::info("MH_Initialize");

    Options::Initialize(mod);
    auto& options = Options::Get();

    if (!options.IsCyberpunk2077())
        return true;

    if(options.PatchSMT)
        SmtAmdPatch(&options.GameImage);

    if (options.PatchAVX && options.GameImage.version <= Image::MakeVersion(1, 4))
        PatchAvx(&options.GameImage);

    if(options.PatchMemoryPool && options.GameImage.version <= Image::MakeVersion(1,4))
        PoolPatch(&options.GameImage);

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
        Overlay::Get().m_enabled = true;
        std::thread t([]()
            {
                if (kiero::init(kiero::RenderType::D3D12) != kiero::Status::Success)
                {
                    spdlog::error("Kiero failed!");
                    Options::Get().Uninject = true;
                }
                else
                    Overlay::Get().Hook();
            });
        t.detach();
    }

    spdlog::default_logger()->flush();

    do {
        Sleep(100);
    } while (!Options::Get().Uninject == true);

    Overlay::release();

    kiero::MH_shutdown();

    Overlay::InputHookRemove();

    Overlay::Shutdown();

    spdlog::info("Bye Bye !!!");
    spdlog::drop("");

    Beep(220, 100);
    FreeLibraryAndExitThread(mod, 0);

    return 0;
}


BOOL APIENTRY DllMain(HMODULE mod, DWORD ul_reason_for_call, LPVOID) {
    switch(ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Initialize, mod, 0, 0);       
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}
