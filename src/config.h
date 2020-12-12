#pragma once
//
// If you want to use "plugin" system, define the following macro USE_VERSION_DLL_HOOK_PLUGIN.
// When you define this macro, pseudo version.dll will load DLLs from the following path (recursively, dictionary order).
//    <PATH-OF-YOUR-version.dll>\version.dll.plugins\**.dll
// All native DLLs are loaded by LoadLibrary(), and unloaded by FreeLibrary().
//
// Unless, comment out the following line.  All plugin system completely removed.
//
#define USE_VERSION_DLL_HOOK_PLUGIN 1


//
// If you want to use MinHook, define the following macro USE_MINHOOK.
//
#define USE_MINHOOK 1


#if defined(_DEBUG)
#define USE_OUTOUT_DEBUG_STRING 1
#define USE_DEBUG_TRACE 1
#else
#define USE_OUTOUT_DEBUG_STRING 0
#define USE_DEBUG_TRACE 0
#endif
