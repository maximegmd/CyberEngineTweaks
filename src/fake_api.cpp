#include "common.hpp"

#define DllExport extern "C" __declspec(dllexport)

DllExport const wchar_t* VersionDll_Test() {
    DEBUG_TRACE("Version.dll : VersionDll_Test");
    return L"Version.dll : VersionDll_Test";
}

DllExport const wchar_t* VersionDll_Test2() {
    DEBUG_TRACE("Version.dll : VersionDll_Test2");
    return L"Version.dll : VersionDll_Test2";
}

DllExport const wchar_t* FAKE_VERSION_DLL_SIGNATURE() {
    return L"You're calling FAKE version.dll";
}
