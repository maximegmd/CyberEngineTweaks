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

#pragma region CRenderGlobal
constexpr uintptr_t CRenderGlobal_InstanceOffset = 0x144C91230 - ImageBase; // 48 8B 05 ? ? ? ? 8B D1 48 8B 88 18 8A 5A 01, expected: 1, index: 0, offset: 3
constexpr uintptr_t CRenderGlobal__DoNotUse_RenderQueueOffset = 0x18BE22C76 - ImageBase; // 4D 8B 0E 49 39 31 0F 84 85 00 00 00 41 39 71 24 74 ? 49 8B 95, expected: 1, index: 0, offset: 0
constexpr uintptr_t CRenderGlobal_Resize = 0x142D41700 - ImageBase; // 44 88 4C 24 20 44 89 44 24 18 89 54 24 10 89 4C, expected: 1, index: 0
#pragma endregion

#pragma region CRenderNode_Present
constexpr uintptr_t CRenderNode_Present_DoInternal = 0x142D439E0 - ImageBase; // 48 89 5C 24 08 48 89 6C  24 18 48 89 74 24 20 57 41 56 41 57 48 83 EC 30  8B 01 41 8B F8 4C 8B 35, expected: 1, index: 0
#pragma endregion
} // namespace CyberEngineTweaks::Addresses
