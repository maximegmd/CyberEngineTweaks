#include "common.hpp"
#include "version_dll.hpp"
#include "lz32_dll.hpp"
#include "plugin.hpp"
#include "minhook_api.hpp"
#include <array>
#include <mutex>

namespace {
wchar_t moduleFullpathFilename[MAX_PATH + 1];
wchar_t drive[_MAX_DRIVE+1];
wchar_t dir[_MAX_DIR+1];
wchar_t fname[_MAX_FNAME+1];
wchar_t ext[_MAX_EXT+1];

wchar_t systemDirectory[MAX_PATH + 1];

bool isWin64() {
#if defined(_WIN64)
    DEBUG_TRACE(L"isWin64() : _WIN64");
    return true;
#else
    DEBUG_TRACE(L"isWin64() : _WIN32");
    BOOL wow64Process = FALSE;
    if(IsWow64Process(GetCurrentProcess(), &wow64Process) == TRUE) {
        DEBUG_TRACE(L"IsWow64Process() : TRUE -> %d", wow64Process);
        return wow64Process == TRUE;
    } else {
        DEBUG_TRACE(L"IsWow64Process() : FALSE");
        return false;
    }
#endif
}

bool isVersionDll() {
    return _wcsicmp(fname, L"version") == 0;
}

bool isLz32Dll() {
    return _wcsicmp(fname, L"lz32") == 0;
}

void init(HMODULE hModule) {
    DEBUG_TRACE(L"init(hModule=%p) - begin", hModule);

    if(isWin64()) {
        DEBUG_TRACE(L"init(hModule=%p) : isWin64 == true");
        GetSystemDirectoryW(systemDirectory, static_cast<UINT>(sizeof(systemDirectory)/sizeof(systemDirectory[0])));
    } else {
        DEBUG_TRACE(L"init(hModule=%p) : isWin64 == false");
        GetSystemWow64DirectoryW(systemDirectory, static_cast<UINT>(sizeof(systemDirectory)/sizeof(systemDirectory[0])));
    }
    DEBUG_TRACE(L"init : systemDirectory=[%s]", systemDirectory);

    GetModuleFileNameW(hModule, moduleFullpathFilename, static_cast<UINT>(sizeof(moduleFullpathFilename)/sizeof(moduleFullpathFilename[0])));
    DEBUG_TRACE(L"init : moduleFullpathFilename=[%s]", moduleFullpathFilename);

    _wsplitpath_s(moduleFullpathFilename, drive, dir, fname, ext);
    DEBUG_TRACE(L"init : fname=[%s]", fname);

    SetEnvironmentVariableW(L"VERSION_DLL_PLUGIN_PROVIDER", moduleFullpathFilename);

    DEBUG_TRACE(L"DLL_PROCESS_ATTACH (hModule=%p)", hModule);
    minhook_api::init();
    if(isVersionDll()) {
        version_dll::loadGenuineDll(systemDirectory);
    }
    if(isLz32Dll()) {
        lz32_dll::loadGenuineDll(systemDirectory);
    }
    plugin::loadPluginDlls();
    {
        //
        // *** You can put your own startup code here ***
        //
    }

    DEBUG_TRACE(L"init(hModule=%p) - end", hModule);
}

void cleanup() {
    {
        //
        // *** You can put your own cleanup code here ***
        //
    }
    plugin::unloadPluginDlls();
    if(isLz32Dll()) {
        lz32_dll::unloadGenuineDll();
    }
    if(isVersionDll()) {
        version_dll::unloadGenuineDll();
    }
    minhook_api::cleanup();
    DEBUG_TRACE(L"DLL_PROCESS_DETACH");
}

}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
    static std::once_flag initFlag;
    static std::once_flag cleanupFlag;

    switch(ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        std::call_once(initFlag, [&]() { init(hModule); });
        break;

    case DLL_PROCESS_DETACH:
        std::call_once(cleanupFlag, [&]() { cleanup(); });
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }

    return TRUE;
}
