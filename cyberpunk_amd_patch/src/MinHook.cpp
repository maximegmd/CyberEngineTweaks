#include "MinHook.hpp"
#include "VersionDllPluginProvider.hpp"
#include <stdio.h>

namespace MinHookApi {
//  decltype(&Decl::MH_Initialize       ) MH_Initialize         = nullptr;
//  decltype(&Decl::MH_Uninitialize     ) MH_Uninitialize       = nullptr;
    decltype(&Decl::MH_CreateHook       ) MH_CreateHook         = nullptr;
    decltype(&Decl::MH_CreateHookApi    ) MH_CreateHookApi      = nullptr;
    decltype(&Decl::MH_CreateHookApiEx  ) MH_CreateHookApiEx    = nullptr;
    decltype(&Decl::MH_RemoveHook       ) MH_RemoveHook         = nullptr;
    decltype(&Decl::MH_EnableHook       ) MH_EnableHook         = nullptr;
    decltype(&Decl::MH_DisableHook      ) MH_DisableHook        = nullptr;
    decltype(&Decl::MH_QueueEnableHook  ) MH_QueueEnableHook    = nullptr;
    decltype(&Decl::MH_QueueDisableHook ) MH_QueueDisableHook   = nullptr;
    decltype(&Decl::MH_ApplyQueued      ) MH_ApplyQueued        = nullptr;
    decltype(&Decl::MH_StatusToString   ) MH_StatusToString     = nullptr;

    MH_STATUS MH_Initialize() {
        HMODULE h = VersionDll::PluginProvider::getDllHmodule();
        if(h == nullptr) {
            return MH_ERROR_NOT_INITIALIZED;
        }

//      * reinterpret_cast<void**>(&MH_Initialize       ) = GetProcAddress(h, "MH_Initialize");
//      * reinterpret_cast<void**>(&MH_Uninitialize     ) = GetProcAddress(h, "MH_Uninitialize");
        * reinterpret_cast<void**>(&MH_CreateHook       ) = GetProcAddress(h, "MH_CreateHook_");
        * reinterpret_cast<void**>(&MH_CreateHookApi    ) = GetProcAddress(h, "MH_CreateHookApi_");
        * reinterpret_cast<void**>(&MH_CreateHookApiEx  ) = GetProcAddress(h, "MH_CreateHookApiEx_");
        * reinterpret_cast<void**>(&MH_RemoveHook       ) = GetProcAddress(h, "MH_RemoveHook_");
        * reinterpret_cast<void**>(&MH_EnableHook       ) = GetProcAddress(h, "MH_EnableHook_");
        * reinterpret_cast<void**>(&MH_DisableHook      ) = GetProcAddress(h, "MH_DisableHook_");
        * reinterpret_cast<void**>(&MH_QueueEnableHook  ) = GetProcAddress(h, "MH_QueueEnableHook_");
        * reinterpret_cast<void**>(&MH_QueueDisableHook ) = GetProcAddress(h, "MH_QueueDisableHook_");
        * reinterpret_cast<void**>(&MH_ApplyQueued      ) = GetProcAddress(h, "MH_ApplyQueued_");
        * reinterpret_cast<void**>(&MH_StatusToString   ) = GetProcAddress(h, "MH_StatusToString_");
        return MH_OK;
    }

    MH_STATUS MH_Uninitialize() {
        return MH_OK;
    }
}
