#pragma once

/*
 * This file is generated. DO NOT modify it!
 *
 * Add new patterns in "patterns.py" file located in "project_root/scripts" and run "find_patterns.py".
 * The new file should be located in "idb_path/Addresses.h".
 */
#include <cstdint>

// Addresses for Cyberpunk 2077, version 1.61.
namespace CyberEngineTweaks::Addresses
{
constexpr uintptr_t ImageBase = 0x140000000;

#pragma region CBaseInitializationState
constexpr uintptr_t CBaseInitializationState_OnTick = 0x140A72BA0 - ImageBase; // 48 83 EC 28 48 8B 05 ? ? ? ? 4C 8B C2 48 85 C0 75 12 8D 50 03 49 8B
                                                                               // C8 E8 ? ? ? ?, expected: 1, index: 0
#pragma endregion

#pragma region CGame
constexpr uintptr_t CGame_Main = 0x140A750D0 - ImageBase; // 40 57 48 83 EC 70 48 8B F9 0F 29 7C 24 50 48 8D 4C 24 38, expected: 1, index: 0
#pragma endregion

#pragma region CGameApplication
constexpr uintptr_t CGameApplication_Run = 0x140A712D0 - ImageBase; // 48 89 5C 24 08 57 48 83 EC 20 48 8B D9 33 FF 90 E8 ? ? ? ? 84 C0, expected: 1, index: 0
#pragma endregion

#pragma region CInitializationState
constexpr uintptr_t CInitializationState_OnTick = 0x140A72EA0 - ImageBase; // 48 83 EC 28 48 8B 05 ? ? ? ? 4C 8B C2 8B 88 F8 00 00 00 85 C9 74 3D 83
                                                                           // E9 02 74 25 83 E9 01, expected: 1, index: 0
#pragma endregion

#pragma region CPatches
constexpr uintptr_t CPatches_BoundaryTeleport = 0x141B2BE80 - ImageBase; // 48 8B C4 55 53 41 54 48  8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 89 70 10 48 8D 59 48, expected: 1, index: 0
constexpr uintptr_t CPatches_IntroMovie = 0x1401FF080 - ImageBase;       // 48 89 5C 24 08 57 48 83 EC 20 48 8B 44 24 50 48 8B D9 48 89 41 08, expected: 1, index: 0
constexpr uintptr_t CPatches_Vignette = 0x14117C480 - ImageBase;         // 48 8B 41 30 48 83 78 68 00 74, expected: 1, index: 0
constexpr uintptr_t CPatches_MinimapFlicker = 0x1425AAB5D - ImageBase;   // 83 79 2C 00 48 8B F2 4C, expected: 1, index: 0
constexpr uintptr_t CPatches_OptionsInit = 0x142BF2600 - ImageBase;      // 40 53 48 83 EC 40 48 8B D9 48 8D 4C 24 20 E8 ? ? ? ? E8 ? ? ? ? 4C 8B 43 08, expected: 1, index: 0
constexpr uintptr_t CPatches_SkipStartScreen = 0x142A25180 - ImageBase;  // 74 5F E8 ? ? ? ? 48 8D 4C 24 20 8B D8 E8 ? ? ? ? 48 8B C8 8B D3 E8, expected: 2, index: 1
constexpr uintptr_t CPatches_AmdSMT = 0x142B9B28B - ImageBase;           // 75 2D 33 C9 B8 01 00 00 00 0F A2 8B C8 C1 F9 08, expected: 1, index: 0
#pragma endregion

#pragma region CPhotoMode
constexpr uintptr_t CPhotoMode_SetRecordID = 0x142DC0CC0 - ImageBase; // 48 8B C4 55 57 48 8D 68 A1 48 81 EC 98 00 00 00 48 89 58 08 48 8B D9 48 89 70
                                                                      // 18 48 8D 4D 27 48, expected: 1, index: 0
#pragma endregion

#pragma region CRenderGlobal
constexpr uintptr_t CRenderGlobal_InstanceOffset = 0x144DDDDF0 - ImageBase; // 48 89 5C 24 08 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 30
                                                                            // 8B 01 41 8B F8 4C 8B 35, expected: 1, index: 0, offset: 32
constexpr uintptr_t CRenderGlobal__DoNotUse_RenderQueueOffset =
    0x151F6D589 - ImageBase;                                          // 49 39 29 0F 84 ? ? ? ? 41 39 69 24 0F 84 ? ? ? ? 49 8B 95, expected: 1, index: 0, offset: 0
constexpr uintptr_t CRenderGlobal_Resize = 0x142CE1EA0 - ImageBase;   // 44 88 4C 24 20 44 89 44 24 18 89 54 24 10 89 4C, expected: 1, index: 0
constexpr uintptr_t CRenderGlobal_Shutdown = 0x142CCC550 - ImageBase; // 48 89 6C 24 20 41 56 48 83 EC 20 48 8D 05, expected: 1, index: 0
#pragma endregion

#pragma region CRenderNode_Present
constexpr uintptr_t CRenderNode_Present_DoInternal = 0x142CE4450 - ImageBase; // 48 89 5C 24 08 48 89 6C 24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC
                                                                              // 30 8B 01 41 8B F8 4C 8B 35, expected: 1, index: 0
#pragma endregion

#pragma region CRunningState
constexpr uintptr_t CRunningState_OnTick = 0x140A72FE0 - ImageBase; // 40 53 48 83 EC 20 48 8B 0D ? ? ? ? 48 8B DA E8 ? ? ? ? 84 C0, expected: 1, index: 0
#pragma endregion

#pragma region CScript
constexpr uintptr_t CScript_RunPureScript = 0x14020B9C0 - ImageBase;          // 40 55 48 81 EC D0 00 00 00 48 8D 6C 24 40 8B, expected: 1, index: 0
constexpr uintptr_t CScript_AllocateFunction = 0x1401AC0F0 - ImageBase;       // BA B8 00 00 00 48 8D 4D D7 E8, expected: 3, index: 0
constexpr uintptr_t CScript_Log = 0x1401EDC40 - ImageBase;                    // 40 53 48 83 EC ? 48 8D 4C 24 20 48 8B DA E8 ? ? ? ? 33 D2 48 8D 4C  24 40 E8, expected: 1, index: 0
constexpr uintptr_t CScript_ToStringDEBUG = 0x140BD5400 - ImageBase;          // 48 89 5C 24 08 57 48 83  EC 20 FE 42 62 4C 8D 15 ? ? ? ? 33 C9 33 C0, expected: 4, index: 2
constexpr uintptr_t CScript_LogChannel = 0x1401EDCE0 - ImageBase;             // 4C 8B DC 49 89 5B 08 49  89 73 18 57 48 83 EC 70 48 8B 02 ? ? ? ? ? ? ? FE 42 62
                                                                              // 4D 8D 43 10 33 FF 45 33 C9 49 89  7B 10 48 8B DA 48 89 7A, expected: 1, index: 0
constexpr uintptr_t CScript_TDBIDConstructorDerive = 0x142BEB670 - ImageBase; // 40 53 48 83 EC 30 33 C0 4C 89 44 24 20 48 8B DA, expected: 1, index: 0
constexpr uintptr_t CScript_TranslateBytecode = 0x14027B0D0 - ImageBase;      // 4C 8B DC 55 53 57 41 55 49 8D 6B A1 48 81 EC 98 00 00 00 48 8B 1A 4C 8B E9
                                                                              // 8B 42 0C 48 8D 3C C3, expected: 1, index: 0
constexpr uintptr_t CScript_TweakDBLoad = 0x140BD3730 - ImageBase;            // 48 89 5C 24 18 55 57 41 56 48 8B EC 48 83 EC 70 48 8B D9 45 33 F6 48 8D, expected: 1, index: 0
constexpr uintptr_t CScript_RegisterMemberFunction = 0x14020ACB0 - ImageBase; // 48 89 5C 24 08 57 48 83 EC 20 49 8B C1 4D 8B D0 44 8B 4C 24 58 48 8B
                                                                              // DA 41 83 C9 03, expected: 1, index: 0
#pragma endregion

#pragma region CShutdownState
constexpr uintptr_t CShutdownState_OnTick = 0x140A730F0 - ImageBase; // 48 89 6C 24 18 56 48 83 EC 30 48 8B 0D ? ? ? ?, expected: 1, index: 0
#pragma endregion

#pragma region CWinapi
constexpr uintptr_t CWinapi_ClipToCenter = 0x14078BE70 - ImageBase; // 48 89 5C 24 08 57 48 83 EC 30 48 8B 99 ? 01 00 00 48 8B F9 FF, expected: 1, index: 0
#pragma endregion

#pragma region gameIGameSystem
constexpr uintptr_t gameIGameSystem_Constructor = 0x140AF6356 - ImageBase;   // 48 8B D9 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 C7 43 40 00 00 00 00, expected: 2, index: 0
constexpr uintptr_t gameIGameSystem_Initialize = 0x142DC26E0 - ImageBase;    // 48 89 5C 24 18 48 89 6C 24 20 57 48 83 EC 30 48 8B 42 78, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_UnInitialize = 0x142DC1350 - ImageBase;  // 40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 33 C0 48 89 43 50 48 89 43 48, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_Spawn = 0x142DC38E0 - ImageBase;         // 48 89 5C 24 18 55 56 41 54 41 56 41 57 48 8D 6C 24 90 48 81 EC 70 01 00 00 48
                                                                             // 83 79 50 00 49 8B, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_Despawn = 0x142DC1380 - ImageBase;       // 48 89 5C 24 10 48 89 6C  24 18 56 57 41 54 41 56 41 57 48 83 EC 50 4C 8B F9
                                                                             // 0F 57 C0 48 83 C1 41, expected: 1, index: 0
constexpr uintptr_t gameIGameSystem_SpawnCallback = 0x1410F1E70 - ImageBase; // 48 89 5C 24 18 48 89 6C  24 20 56 57 41 56 48 83 EC 70 48 8B F1 48 8B
                                                                             // EA  48 83 C1 48 E8, expected: 1, index: 0
#pragma endregion

#pragma region PlayerSystem
constexpr uintptr_t PlayerSystem_OnPlayerSpawned = 0x14271DB50 - ImageBase; // 48 8B C4 4C 89 48 20 55 56 57 48 8B EC 48 81 EC 80 00 00 00, expected: 1, index: 0
#pragma endregion
} // namespace CyberEngineTweaks::Addresses
