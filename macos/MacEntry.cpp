// CET macOS: RED4ext plugin entry points. On Windows CET starts from DllMain when the ASI loader maps it; on macOS the RED4ext
// loader loads it as a plugin and calls Main(Load) early, before scripts and TweakDB load. Initialization itself stays in
// src/dllmain.cpp (CetMacInitialize / CetMacShutdown, the Apple branch of its Initialize / Shutdown).

#include <RED4ext/RED4ext.hpp>

#include "MacShims.hpp"

void CetMacInitialize();
void CetMacShutdown();

RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::v1::PluginHandle aHandle, RED4ext::v1::EMainReason aReason, const RED4ext::v1::Sdk* aSdk)
{
    switch (aReason)
    {
    case RED4ext::v1::EMainReason::Load:
    {
        CetMac::SetPlugin(aSdk, aHandle);
        if (aSdk && aSdk->runtime)
        {
            CetMac::SetRuntimeVersion(aSdk->runtime->major, aSdk->runtime->minor, aSdk->runtime->patch);
        }
        CetMac::Log("[cet] Cyber Engine Tweaks for macOS loading (game %s, plugin folder %s)", CetMac::RuntimeVersionString(),
                    CetMac::PluginDir().c_str());
        CetMacInitialize();
        break;
    }
    case RED4ext::v1::EMainReason::Unload:
    {
        CetMacShutdown();
        break;
    }
    }
    return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::v1::PluginInfo* aInfo)
{
    aInfo->name = L"Cyber Engine Tweaks";
    aInfo->author = L"Cyber Engine Tweaks Team (macOS port: codeman101)";
    aInfo->version = RED4EXT_V1_SEMVER(1, 38, 0);
    aInfo->runtime = RED4EXT_V1_RUNTIME_VERSION_INDEPENDENT;
    aInfo->sdk = RED4EXT_V1_SDK_VERSION_1_0_0_COMPAT_0_5_0; // loadable by v0 loaders too
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    return RED4EXT_API_VERSION_1_COMPAT_0; // the API v0 number: older loaders load it, v1 loaders treat it as v1
}
