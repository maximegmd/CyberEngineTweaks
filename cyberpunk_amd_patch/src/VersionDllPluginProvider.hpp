#pragma once
#include <tchar.h>

namespace VersionDll { namespace PluginProvider {

inline const wchar_t* getEnvironmentVariableName() {
    return L"VERSION_DLL_PLUGIN_PROVIDER";
}

inline HMODULE getDllHmodule() {
	HMODULE h = nullptr;

    const auto* name = getEnvironmentVariableName();
    const size_t maxLength = MAX_PATH+1;

    wchar_t pluginProvider[maxLength];
    if(0 == GetEnvironmentVariableW(name, pluginProvider, maxLength)) {
        return nullptr;
    }

    if(! GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, pluginProvider, &h)) {
        return nullptr;
    }

    return h;
}

}} // namespace VersionDll::PluginProvider
