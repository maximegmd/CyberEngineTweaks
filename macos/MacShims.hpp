#pragma once

// CET macOS: the macOS layer of Cyber Engine Tweaks. Everything Windows-only in CET has an __APPLE__ branch that calls here.
// Pattern: let_there_be_flight/macos/MacShims.hpp.

#include <cstdint>
#include <filesystem>

#include <RED4ext/Api/PluginHandle.hpp>
#include <RED4ext/GameEngine.hpp>
#include <RED4ext/Api/Sdk.hpp>

extern "C"
{
// Fail-safe for an address hash with no Mac row: logs the hash once and returns a stub that logs its first call and returns 0.
uintptr_t cet_mac_trap_for_hash(uint32_t aHash);
}

namespace CetMac
{
// The game image base (the MH_EXECUTE header).
uintptr_t ImageBase();

// The span of the game image in memory (from the Mach-O header to the end of the last segment, slide included).
uintptr_t ImageSize();

// ImageBase() + aRva when the instruction words there equal aExpected (aCount words), else 0 with a log line naming aName.
uintptr_t VerifiedAddress(uintptr_t aRva, const uint32_t* aExpected, size_t aCount, const char* aName);

// True for a target FlushDeferredHooks handles (see above).
bool IsSharedTarget(uintptr_t aAddress);

// True when the address is the trap stub (an unmapped hash) or 0.
bool IsTrap(uintptr_t aAddress);

// The plugin folder (red4ext/plugins/cyber_engine_tweaks) and the game root, both through dladdr.
std::filesystem::path PluginDir();
std::filesystem::path RootDir();
// The game executable (…/Cyberpunk2077.app/Contents/MacOS/Cyberpunk2077).
std::filesystem::path ExePath();

// The GameInstance, from Codeware's codeware_mac_game_instance (null until the game has one, or without Codeware). The SDK path
// CGameEngine::Get()->framework->gameInstance does not hold on macOS: the engine global is not mapped (Codeware captures the engine in
// a hook) and the framework sits at engine +0x338, not the SDK's +0x308 (cp2077-codeware/macos/MacEngine.cpp).
RED4ext::GameInstance* GameInstance();

// Writes a line to stderr (the launcher log) and, once CET's logger exists, to cyber_engine_tweaks.log.
void Log(const char* aFormat, ...) __attribute__((format(printf, 1, 2)));

// The plugin's RED4ext handle and SDK, saved by Main(Load).
void SetPlugin(const RED4ext::Sdk* aSdk, RED4ext::PluginHandle aHandle);
const RED4ext::Sdk* PluginSdk();
RED4ext::PluginHandle PluginHandleValue();
// The game version RED4ext reports (Query/Main), e.g. "2.3.1".
void SetRuntimeVersion(uint8_t aMajor, uint16_t aMinor, uint32_t aPatch);
const char* RuntimeVersionString();

enum class HookResult
{
    Deferred, // a target another plugin owns: queued for FlushDeferredHooks
    Installed,
    Chained,
    NotActive, // RED4ext-macos accepted the attach but the offset is not on RED4EXT_GUM_HOOK_OFFSETS (no-op)
    Refused,   // no Mac address, or a peer owns the target and refused the chain
    Failed
};
const char* HookResultName(HookResult aResult);

// One hook attach attempt with a log line. Asks Codeware and ArchiveXL first (a plugin that does not own the target answers
// false), then RED4ext once. *aOriginal is null unless the result is Installed or Chained. CET_MAC_NOHOOKS=all or a comma list of
// hook names leaves hooks out (bisecting).
HookResult AttachHook(uintptr_t aTarget, void* aDetour, void** aOriginal, const char* aName);
// Targets another macOS plugin hooks too (RED4ext for macOS takes one hook per target): TweakDB::LoadOptimized (ArchiveXL) and
// PlayerSystem::OnPlayerSpawned (Codeware). RED4ext loads the plugins in folder order, so CET can load before them; AttachHook only
// queues these, and FlushDeferredHooks (called from every game-state tick until it returns true) chains them through the owner once
// every plugin is loaded, or attaches directly when the owner is not installed. *aOriginal is written when the hook is live.
bool FlushDeferredHooks();
// True once CetMacInitialize has finished without an exception.
void SetCetReady(bool aReady);
bool CetReady();

// The log name of a hook target (the address hash's name when CET knows it, else the RVA).
const char* HookName(uintptr_t aTarget);
} // namespace CetMac
