#pragma once

/*
 * This file is generated. DO NOT modify it!
 *
 * Add new patterns in "patterns.py" file located in "project_root/scripts" and run "find_patterns.py".
 * The new file should be located in "idb_path/Addresses.hpp".
 */
#include <cstdint>

// Addresses for Cyberpunk 2077, version 1.31.
namespace CyberEngineTweaks::Addresses
{
constexpr uintptr_t ImageBase = 0x140000000;

#pragma region CGame
constexpr uintptr_t CGame_Main = 0x140AD86C0 - ImageBase; // 40 55 57 41 57 48 81 EC, expected: 1, index: 0
#pragma endregion

#pragma region CPatches
constexpr uintptr_t CPatches_BoundaryTeleport = 0x141BBDED0 - ImageBase; // 48 8B C4 55 53 41 54 48 8D A8 78, expected: 1, index: 0
constexpr uintptr_t CPatches_IntroMovie = 0x140222200 - ImageBase; // 48 89 5C 24 08 57 48 83 EC 20 48 8B 44 24 50 48 8B D9 48 89 41 08, expected: 1, index: 0
constexpr uintptr_t CPatches_Vignette = 0x1412233C0 - ImageBase; // 48 8B 41 30 48 83 78 68 00 74, expected: 1, index: 0
constexpr uintptr_t CPatches_IsFinal = 0x14020DDFD - ImageBase; // 48 BB 87 C9 B1 63 33 01 15 75, expected: 1, index: 0
constexpr uintptr_t CPatches_CanDebugTeleport = 0x1426C0756 - ImageBase; // 48 BB C3 63 E3 32 7C A2 3C C1, expected: 1, index: 0
constexpr uintptr_t CPatches_MinimapFlicker = 0x14260AD98 - ImageBase; // 83 79 2C 00 48 8B F2 4C, expected: 1, index: 0
constexpr uintptr_t CPatches_OptionsInit = 0x142C64300 - ImageBase; // 48 89 5C 24 08 48 89 74 24 10 57 48 83 EC 40 48 8B F1 48 8D 4C 24 20 E8, expected: 1, index: 0
constexpr uintptr_t CPatches_RemovePedestrians = 0x141E98965 - ImageBase; // 3B D8 0F 4E C3 8B D8 85 DB 0F 8E, expected: 1, index: 0
constexpr uintptr_t CPatches_SkipStartScreen = 0x142A50634 - ImageBase; // 48 BB E6 F8 A5 A3 36 56 4E A7 C6 85 B0 ? ? ? 01, expected: 1, index: 0
constexpr uintptr_t CPatches_AmdSMT = 0x142C1077B - ImageBase; // 75 2D 33 C9 B8 01 00 00 00 0F A2 8B C8 C1 F9 08, expected: 1, index: 0
#pragma endregion

#pragma region CPhotoMode
constexpr uintptr_t CPhotoMode_SetRecordID = 0x142E287C0 - ImageBase; // 48 89 5C 24 08 48 89 74 24 18 55 57 41 56 48 8D 6C 24 B9 48 81 EC 90 00 00 00 48 8B F9, expected: 1, index: 0
#pragma endregion

#pragma region CRenderGlobal
constexpr uintptr_t CRenderGlobal_InstanceOffset = 0x144C91230 - ImageBase; // 48 8B 05 ? ? ? ? 8B D1 48 8B 88 18 8A 5A 01, expected: 1, index: 0, offset: 3
constexpr uintptr_t CRenderGlobal__DoNotUse_RenderQueueOffset = 0x18BE22C76 - ImageBase; // 4D 8B 0E 49 39 31 0F 84 85 00 00 00 41 39 71 24 74 ? 49 8B 95, expected: 1, index: 0, offset: 0
constexpr uintptr_t CRenderGlobal_Resize = 0x142D41700 - ImageBase; // 44 88 4C 24 20 44 89 44 24 18 89 54 24 10 89 4C, expected: 1, index: 0
#pragma endregion

#pragma region CRenderNode_Present
constexpr uintptr_t CRenderNode_Present_DoInternal = 0x142D439E0 - ImageBase; // 48 89 5C 24 08 48 89 6C  24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 30 8B 01 41 8B F8 4C 8B 35, expected: 1, index: 0
#pragma endregion

#pragma region CScript
constexpr uintptr_t CScript_RunPureScript = 0x14022E980 - ImageBase; // 40 55 48 81 EC D0 00 00 00 48 8D 6C 24 40 8B, expected: 1, index: 0
constexpr uintptr_t CScript_CreateFunction = 0x14029CDD0 - ImageBase; // 48 89 5C 24 08 57 48 83 EC 40 8B F9 48 8D 54 24 30 48 8B 0D ? ? ? ? 41 B8 B8 00 00 00, expected: 1, index: 0
constexpr uintptr_t CScript_Log = 0x140210470 - ImageBase; // 40 53 48 83 EC ? 48 8D 4C 24 20 48 8B DA E8 ? ? ? ? 33 D2 48 8D 4C  24 40 E8, expected: 1, index: 0
constexpr uintptr_t CScript_LogChannel = 0x140210470 - ImageBase; // 40 53 48 83 EC ? 48 8D 4C 24 20 48 8B DA E8 ? ? ? ? 33 D2 48 8D 4C 24 40 E8, expected: 1, index: 0
constexpr uintptr_t CScript_TDBIDConstructorDerive = 0x142C5D2B0 - ImageBase; // 40 53 48 83 EC 30 33 C0 4C 89 44 24 20 48 8B DA, expected: 1, index: 0
constexpr uintptr_t CScript_ProcessRunningState = 0x140AD62A0 - ImageBase; // 40 53 48 83 EC 20 48 8B 0D ? ? ? ? 48 8B DA E8 ? ? ? ? 84 C0, expected: 1, index: 0
constexpr uintptr_t CScript_TweakDBLoad = 0x140C43060 - ImageBase; // 48 89 5C 24 10 48 89 7C 24 18 4C 89 74 24 20 55 48 8B EC 48 83 EC 70 48, expected: 1, index: 0
#pragma endregion

#pragma region CWinapi
constexpr uintptr_t CWinapi_ClipToCenter = 0x1407E7DA0 - ImageBase; // 48 89 5C 24 08 57 48 83 EC 30 48 8B 99 ? 01 00 00 48 8B F9 FF, expected: 1, index: 0
#pragma endregion

#pragma region gameIGameSystem
constexpr uintptr_t gameIGameSystem_Constructor = 0x140B4E636 - ImageBase; // 48 8B D9 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 C7 43 40 00 00 00 00, expected: 2, index: 0
constexpr uintptr_t gameIGameSystem_Initialize = 0x142E2A0F0 - ImageBase; // 48 89 5C 24 18 48 89 6C 24 20 57 48 83 EC 30 48 8B 42 78, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_UnInitialize = 0x142E28DE0 - ImageBase; // 40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 33 C0 48 89 43 50 48 89 43 48, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_Spawn = 0x142E2B225 - ImageBase; // FF 90 A8 01 00 00 48 8B 00 4C 8D 85 80 00 00 00, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_Despawn = 0x142E28E80 - ImageBase; // 40 55 53 56 57 41 55 41 56 41 57 48 8B EC 48 83 EC 50, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_SpawnCallback = 0x14119D70E - ImageBase; // 41 57 48 83 EC 70 48 8B E9 4C 8B FA, expected: 1, index: 0
#pragma endregion
} // namespace CyberEngineTweaks::Addresses
