#pragma once

// CET macOS: a MinHook-compatible front for CET's 17 hook sites. On macOS every hook goes through the RED4ext loader's hooking
// interface (Frida GUM, offsets on the allowlist), one attempt each, with a log line (macos/MacShims.cpp). MH_CreateHook installs at
// once; MH_EnableHook and MH_DisableHook are accepted no-ops (CET enables right after creating and never disables while running).

typedef enum MH_STATUS
{
    MH_UNKNOWN = -1,
    MH_OK = 0,
    MH_ERROR_ALREADY_INITIALIZED,
    MH_ERROR_NOT_INITIALIZED,
    MH_ERROR_ALREADY_CREATED,
    MH_ERROR_NOT_CREATED,
    MH_ERROR_ENABLED,
    MH_ERROR_DISABLED,
    MH_ERROR_NOT_EXECUTABLE,
    MH_ERROR_UNSUPPORTED_FUNCTION,
    MH_ERROR_MEMORY_ALLOC,
    MH_ERROR_MEMORY_PROTECT,
    MH_ERROR_MODULE_NOT_FOUND,
    MH_ERROR_FUNCTION_NOT_FOUND
} MH_STATUS;

#define MH_ALL_HOOKS nullptr

MH_STATUS MH_Initialize();
MH_STATUS MH_Uninitialize();
// Installs the hook now. MH_OK only when the hook is live (installed or chained); otherwise *ppOriginal stays null.
MH_STATUS MH_CreateHook(void* pTarget, void* pDetour, void** ppOriginal);
MH_STATUS MH_EnableHook(void* pTarget);
MH_STATUS MH_DisableHook(void* pTarget);
MH_STATUS MH_RemoveHook(void* pTarget);
