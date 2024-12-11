#pragma once

#include <cstdint>

namespace CyberEngineTweaks::AddressHashes
{
#pragma region CBaseInitializationState
constexpr uint32_t CBaseInitializationState_OnTick = 4233370276UL; // red::GameAppBaseInitializationState::OnTick
#pragma endregion

#pragma region CGame
constexpr uint32_t CGame_Main = 1852772247UL; // CBaseEngine::ProcessBaseLoopFrame
#pragma endregion

#pragma region CInitializationState
constexpr uint32_t CInitializationState_OnTick = 2447710505UL; // red::GameAppInitializationState::OnTick
#pragma endregion

#pragma region CPatches
constexpr uint32_t CPatches_BoundaryTeleport = 887623293UL; // game::WorldBoundarySystem::Tick
constexpr uint32_t CPatches_IntroMovie = 4056423627UL;      // <UNKNOWN_SYMBOL>
constexpr uint32_t CPatches_Vignette = 1592528795UL;        // effect::TrackItemVignette::IsValid
constexpr uint32_t CPatches_OptionsInit = 4089777341UL;     // Config::IConfigVar::Register
#pragma endregion

#pragma region CPhotoMode
constexpr uint32_t CPhotoMode_SetRecordID = 4241565651UL; // <UNKNOWN_SYMBOL>
#pragma endregion

#pragma region CRenderGlobal
constexpr uint32_t CRenderGlobal_InstanceOffset = 1239944840UL; // <UNKNOWN_SYMBOL>
constexpr uint32_t CRenderGlobal_Resize = 239671909UL;          // GpuApi::ResizeBackbuffer
constexpr uint32_t CRenderGlobal_Shutdown = 3192982283UL;       // <UNKNOWN_SYMBOL>
#pragma endregion

#pragma region CRenderNode_Present
constexpr uint32_t CRenderNode_Present_DoInternal = 2468877568UL; // GpuApi::Present
#pragma endregion

#pragma region CRunningState
constexpr uint32_t CRunningState_OnTick = 3592689218UL; // red::GameAppRunningState::OnTick
#pragma endregion

#pragma region CScript
constexpr uint32_t CScript_RunPureScript = 3791200470UL;         // rtti::Function::InternalCall
constexpr uint32_t CScript_AllocateFunction = 160045886UL;       // <UNKNOWN_SYMBOL>
constexpr uint32_t CScript_Log = 3455393801UL;                   // <UNKNOWN_SYMBOL>
constexpr uint32_t CScript_LogError = 2135235617UL;              // <UNKNOWN_SYMBOL>
constexpr uint32_t CScript_LogWarning = 3222609133UL;            // <UNKNOWN_SYMBOL>
constexpr uint32_t CScript_ToStringDEBUG = 3515162577UL;         // <UNKNOWN_SYMBOL>
constexpr uint32_t CScript_LogChannel = 1663049434UL;            // <UNKNOWN_SYMBOL>
constexpr uint32_t CScript_LogChannelWarning = 2841780134UL;     // <UNKNOWN_SYMBOL>
constexpr uint32_t CScript_TDBIDConstructorDerive = 326438016UL; // <UNKNOWN_SYMBOL>
constexpr uint32_t CScript_TranslateBytecode = 3442875632UL;     // CScriptDataBinder::LoadOpcodes
constexpr uint32_t CScript_TweakDBLoad = 3602585178UL;           // game::data::TweakDB::LoadOptimized
#pragma endregion

#pragma region CShutdownState
constexpr uint32_t CShutdownState_OnTick = 4069332669UL; // red::GameAppShutdownState::OnTick
#pragma endregion

#pragma region CWinapi
constexpr uint32_t CWinapi_ClipToCenter = 261693736UL; // input::InputSystemWin32Base::Update
#pragma endregion

#pragma region gameIGameSystem
constexpr uint32_t gameIGameSystem_Initialize =
    385618721UL; // <UNKNOWN_SYMBOL> -> should probably be 3114931869 (spawn::Set::Initialize) but that implies we do something weird overall with this func atm
                 // The above would require CET changes as that one wants game instance to be passed at a2+80
constexpr uint32_t gameIGameSystem_UnInitialize = 3313306514UL;  // spawn::Set::Deinitialize
constexpr uint32_t gameIGameSystem_Spawn = 2509382878UL;         // spawn::Set::SpawnObject
constexpr uint32_t gameIGameSystem_Despawn = 3168866665UL;       // spawn::Set::DespawnObject
constexpr uint32_t gameIGameSystem_SpawnCallback = 2840271332UL; // world::RuntimeEntityRegistry::RegisterEntity
#pragma endregion

#pragma region PlayerSystem
constexpr uint32_t PlayerSystem_OnPlayerSpawned = 2050111212UL; // cp::PlayerSystem::OnPlayerMainObjectSpawned
#pragma endregion
} // namespace CyberEngineTweaks::AddressHashes