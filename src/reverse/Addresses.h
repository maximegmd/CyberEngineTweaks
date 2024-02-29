#pragma once

#include <cstdint>

namespace CyberEngineTweaks::AddressHashes
{
#pragma region CBaseInitializationState
constexpr uint32_t CBaseInitializationState_OnTick = 2529693960UL;
#pragma endregion

#pragma region CGame
constexpr uint32_t CGame_Main = 1852772247UL;
#pragma endregion

#pragma region CInitializationState
constexpr uint32_t CInitializationState_OnTick = 2447710505UL;
#pragma endregion

#pragma region CPatches
constexpr uint32_t CPatches_BoundaryTeleport = 887623293UL;
constexpr uint32_t CPatches_IntroMovie = 4056423627UL;
constexpr uint32_t CPatches_Vignette = 1592528795UL;
constexpr uint32_t CPatches_OptionsInit = 2920158527UL;
#pragma endregion

#pragma region CPhotoMode
constexpr uint32_t CPhotoMode_SetRecordID = 2826047827UL;
#pragma endregion

#pragma region CRenderGlobal
constexpr uint32_t CRenderGlobal_InstanceOffset = 1239944840UL;
//constexpr uint32_t CRenderGlobal__DoNotUse_RenderQueueOffset = 0x1B5F5FCB0;
constexpr uint32_t CRenderGlobal_Resize = 239671909UL;
constexpr uint32_t CRenderGlobal_Shutdown = 3192982283UL;
#pragma endregion

#pragma region CRenderNode_Present
constexpr uint32_t CRenderNode_Present_DoInternal = 2468877568UL;
#pragma endregion

#pragma region CRunningState
constexpr uint32_t CRunningState_OnTick = 3592689218UL;
#pragma endregion

#pragma region CScript
constexpr uint32_t CScript_RunPureScript = 3791200470UL;
constexpr uint32_t CScript_AllocateFunction = 160045886UL;
constexpr uint32_t CScript_Log = 3455393801UL;
constexpr uint32_t CScript_LogError = 2135235617UL;
constexpr uint32_t CScript_LogWarning = 3222609133UL;
constexpr uint32_t CScript_ToStringDEBUG = 3515162577UL;
constexpr uint32_t CScript_LogChannel = 1663049434UL;
constexpr uint32_t CScript_LogChannelWarning = 2841780134UL;
constexpr uint32_t CScript_TDBIDConstructorDerive = 326438016UL;
constexpr uint32_t CScript_TranslateBytecode = 3442875632UL;
constexpr uint32_t CScript_TweakDBLoad = 3602585178UL;
constexpr uint32_t CScript_RegisterMemberFunction = 592450491UL;
#pragma endregion

#pragma region CShutdownState
constexpr uint32_t CShutdownState_OnTick = 4069332669UL;
#pragma endregion

#pragma region CWinapi
constexpr uint32_t CWinapi_ClipToCenter = 261693736UL;
#pragma endregion

#pragma region gameIGameSystem
constexpr uint32_t gameIGameSystem_Initialize = 385618721UL;
constexpr uint32_t gameIGameSystem_UnInitialize = 3313306514UL;
constexpr uint32_t gameIGameSystem_Spawn = 2509382878UL;
constexpr uint32_t gameIGameSystem_Despawn = 3168866665UL;
constexpr uint32_t gameIGameSystem_SpawnCallback = 2840271332UL;
#pragma endregion

#pragma region PlayerSystem
constexpr uint32_t PlayerSystem_OnPlayerSpawned = 2050111212UL;
#pragma endregion
} // namespace CyberEngineTweaks::Addresses
