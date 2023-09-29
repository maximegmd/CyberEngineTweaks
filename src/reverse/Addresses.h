#pragma once

/*
 * This file is generated. DO NOT modify it!
 *
 * Add new patterns in "patterns.py" file located in "project_root/scripts" and run "find_patterns.py".
 * The new file should be located in "idb_path/Addresses.h".
 */
#include <cstdint>

// Addresses for Cyberpunk 2077, version 2.0.
namespace CyberEngineTweaks::Addresses
{
constexpr uintptr_t ImageBase = 0x140000000;

#pragma region CBaseInitializationState
constexpr uintptr_t CBaseInitializationState_OnTick = 0x14084A6EC - ImageBase; // 40 53 48 83 EC 20 48 8B 05 ? ? ? ? 33 DB 4C 8B C2 48 85 C0 ? ? ? ?, expected: 1, index: 0
#pragma endregion

#pragma region CGame
constexpr uintptr_t CGame_Main =
    0x1402C5960 - ImageBase; // 48 89 5C 24 10 55 56 57 48 8B EC 48 81 EC 80 00 00 00 48 8B F9 0F 29 74 24 70 0F 29 7C 24 60 48 8D 4D C0, expected: 1, index: 0
#pragma endregion

#pragma region CInitializationState
constexpr uintptr_t CInitializationState_OnTick = 0x14084A7A8 - ImageBase; // 40 53 48 83 EC 30 48 8B 05 ? ? ? ? 33 DB 4C 8B C2 8B 88 08 01 00 00, expected: 1, index: 0
#pragma endregion

#pragma region CPatches
constexpr uintptr_t CPatches_BoundaryTeleport =
    0x140C42C10 - ImageBase; // 48 8B C4 48 89 58 10 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 F8 FE FF FF 48 81 EC D0 01 00 00 0F 29 78 B8 48 8D 51 48, expected: 1, index: 0
constexpr uintptr_t CPatches_IntroMovie = 0x14011D85C - ImageBase;  // 48 89 5C 24 08 57 48 83 EC 20 48 8B 44 24 50 48 8B D9 48 89 41 08, expected: 1, index: 0
constexpr uintptr_t CPatches_Vignette = 0x14230A6D4 - ImageBase;    // 33 C0 48 39  41 68 74 11, expected: 1, index: 0
constexpr uintptr_t CPatches_OptionsInit = 0x1402CBCFC - ImageBase; // 48 89 5C 24 08 55 48 8B EC 48 83 EC 70 48 83 65 F8 00 48 8B D9 83 65 F4 00, expected: 1, index: 0
#pragma endregion

#pragma region CPhotoMode
constexpr uintptr_t CPhotoMode_SetRecordID =
    0x140817CE4 - ImageBase; // 48 89 5C 24 10 48 89 4C 24 08 55 48 8B EC 48 83 EC 40 48 8B DA 48 8D 4D E0 48 8D 55 10 E8, expected: 1, index: 0
#pragma endregion

#pragma region CRenderGlobal
constexpr uintptr_t CRenderGlobal_InstanceOffset =
    0x1432FE498 - ImageBase; // 48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 4C 89 48 20 57 41 56 41 57 48 83 EC 30 8B 01 45 8B, expected: 1, index: 0, offset: 36
constexpr uintptr_t CRenderGlobal__DoNotUse_RenderQueueOffset =
    0x1B5EF8520 - ImageBase; // 39 72 24 74 5B 48 8B 4A 18 4C 8D 8C 24 88 00 00 00 8B 42 24 44 8B C7 48 8B 95 ? ? ? ?, expected: 1, index: 0, offset: 0
constexpr uintptr_t CRenderGlobal_Resize =
    0x140908F78 - ImageBase; // 48 8B C4 44 88 48 20 44 89 40 18 89 50 10 89 48 08 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 68 88 48 81 EC 38 01 00 00, expected: 1, index: 0
constexpr uintptr_t CRenderGlobal_Shutdown = 0x141041734 - ImageBase; // 40 53 48 83 EC 20 48 8B D9 48 8D 05 ? ? ? ? 48 81 C1 98 00 00 00 48 89 01 E8, expected: 1, index: 0
#pragma endregion

#pragma region CRenderNode_Present
constexpr uintptr_t CRenderNode_Present_DoInternal =
    0x140655CE0 - ImageBase; // 48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 4C 89 48 20 57 41 56 41 57 48 83 EC 30 8B 01 45 8B, expected: 1, index: 0
#pragma endregion

#pragma region CRunningState
constexpr uintptr_t CRunningState_OnTick = 0x14084A744 - ImageBase; // 40 53 48 83 EC 30 83 64 24 28 00 48 8D 05 ? ? ? ? 48 8B 0D ? ? ? ? 48 8B DA, expected: 1, index: 0
#pragma endregion

#pragma region CScript
constexpr uintptr_t CScript_RunPureScript = 0x14027EB94 - ImageBase;    // 40 55 48 81 EC D0 00 00 00 48 8D 6C 24 40 8B, expected: 1, index: 0
constexpr uintptr_t CScript_AllocateFunction = 0x141F91E6C - ImageBase; // 40 53 48 83 EC 30 BA B8 00 00 00 48 8D 4C 24 20 E8, expected: 2, index: 0
constexpr uintptr_t CScript_Log = 0x140EC5D7C - ImageBase;              // 48 8B C4 53 48 83 EC 70 48 83 60 C0 00 48 8D 48 C8 83 60 BC 00, expected: 3, index: 0
constexpr uintptr_t CScript_LogError = 0x14109ECF8 - ImageBase;         // 48 8B C4 53 48 83 EC 70 48 83 60 C0 00 48 8D 48 C8 83 60 BC 00, expected: 3, index: 1
constexpr uintptr_t CScript_LogWarning = 0x141136F70 - ImageBase;       // 48 8B C4 53 48 83 EC 70 48 83 60 C0 00 48 8D 48 C8 83 60 BC 00, expected: 3, index: 2
constexpr uintptr_t CScript_ToStringDEBUG = 0x140E05224 - ImageBase;    // 48 89 5C 24 08 57 48 83 EC 20 83 64 24 38 00 4C 8D 15 ? ? ? ? FE 42 62 33 C0, expected: 4, index: 1
constexpr uintptr_t CScript_LogChannel = 0x140D01450 - ImageBase; // 48 89 5C 24 08 48 89 74 24 18 55 48 8B EC 48 83 EC 70 48 8B 02 48 8D 35 ? ? ? ? 48 83 65 18 00 4C 8D 45 18 48
                                                                  // 83 62 30 00 45 33 C9 48 83 62 38 00, expected: 2, index: 0
constexpr uintptr_t CScript_LogChannelWarning = 0x141F9E2F8 - ImageBase; // 48 89 5C 24 08 48 89 74 24 18 55 48 8B EC 48 83 EC 70 48 8B 02 48 8D 35 ? ? ? ? 48 83 65 18 00 4C 8D 45
                                                                         // 18 48 83 62 30 00 45 33 C9 48 83 62 38 00, expected: 2, index: 1
constexpr uintptr_t CScript_TDBIDConstructorDerive = 0x1405981F8 - ImageBase; // 48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 45 33 C9 48 8B FA, expected: 1, index: 0
constexpr uintptr_t CScript_TranslateBytecode =
    0x14021CA60 - ImageBase; // 48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B 1A 48 8B E9 8B 42 0C, expected: 2, index: 0
constexpr uintptr_t CScript_TweakDBLoad =
    0x141410CBC - ImageBase; // 48 89 5C 24 10 48 89 7C 24 18 55 48 8B EC 48 83 EC 70 48 8B F9 48 8B DA 48 8B 0D ? ? ? ? 48 8B 01 FF 90 B8 00 00 00, expected: 1, index: 0
constexpr uintptr_t CScript_RegisterMemberFunction =
    0x140ADCB6C - ImageBase; // 40 53 48 83 EC 20 49 8B C1 4D 8B D0 44 8B 4C 24 58 4C 8B DA 41 83 C9 03 4C 8B C0 49 8B D2 48 8B D9 E8, expected: 1, index: 0
#pragma endregion

#pragma region CShutdownState
constexpr uintptr_t CShutdownState_OnTick = 0x140380910 - ImageBase; // 40 53 48 83 EC 20 48 8B DA E8 ? ? ? ? 48 8B CB 89 83 B0 02 00 00 , expected: 1, index: 0
#pragma endregion

#pragma region CWinapi
constexpr uintptr_t CWinapi_ClipToCenter = 0x140297FA8 - ImageBase; // 48 89 5C 24 08 55 48 8B EC 48 83 EC 30 48 8B D9 48 8B 89 68 01 00 00, expected: 1, index: 0
#pragma endregion

#pragma region gameIGameSystem
constexpr uintptr_t gameIGameSystem_Initialize = 0x1407147C0 - ImageBase;   // 48 89 5C 24 08 57 48 83 EC 30 48 8B 42 78 4C 8B CA 48 8B D9, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_UnInitialize = 0x141F96058 - ImageBase; //  48 89 5C 24 10 48 89 74 24 18 57 48 83 EC 20 48 8B F9 48 8D 51 42, expected: 2, index: 1
constexpr uintptr_t gameIGameSystem_Spawn =
    0x140668FEC - ImageBase; // 48 89 5C 24 10 48 89 74 24 18 55 57 41 56 48 8D 6C 24 B0 48 81 EC 50 01 00 00 48 83 79 50 00 49 8B D9 4D 8B F0, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_Despawn =
    0x140578DB4 -
    ImageBase; // 48 8B C4 48  89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41  56 48 83 EC 40 48 8B E9 0F 57 C0 48 83 C1 41 48 8B F2 F3 0F 7F 40 D8 E8, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_SpawnCallback =
    0x140496DB4 - ImageBase; // 48 89 5C 24 10 48 89 6C 24 18 48 89 74 24 20 57 48 83 EC 60  48 8B F1 48 8B FA 48 83 C1 48 E8, expected: 1, index: 0
#pragma endregion

#pragma region PlayerSystem
constexpr uintptr_t PlayerSystem_OnPlayerSpawned =
    0x1409FE5A0 - ImageBase; // 48 89 5C 24 18 48 89 74 24 20 55 57 41 54 41 56 41 57 48 8B EC 48 83 EC 50 48 8B DA 48 8B F9, expected: 1, index: 0
#pragma endregion
} // namespace CyberEngineTweaks::Addresses
