#pragma once

#include "FunctionOverride.h"
#include "ScriptStore.h"

#if GAME_CYBERPUNK
#include "cyberpunk/reverse/RTTIMapper.h"
#include "cyberpunk/reverse/SingletonReference.h"

using CBaseRTTIType = RED4ext::CBaseRTTIType;
using CStackType = RED4ext::CStackType;
#else
#include "witcher3/reverse/RTTIMapper.h"

using CBaseRTTIType = red3lib::IRTTIType;

struct CStackType
{
    CBaseRTTIType* type;
    void* value;
};
#endif

struct D3D12;

struct Scripting
{
    using LockedState = TiltedPhoques::Locked<sol::state, std::recursive_mutex>;

    Scripting(const Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12);
    ~Scripting() = default;

    void Initialize();
    void PostInitializeScripting();
    void PostInitializeTweakDB();
    void PostInitializeMods();

    [[nodiscard]] const VKBind* GetBind(const VKModBind& acModBind) const;
    [[nodiscard]] const TiltedPhoques::Vector<VKBind>* GetBinds(const std::string& acModName) const;
    [[nodiscard]] const TiltedPhoques::Map<std::string, std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>>& GetAllBinds() const;

    void TriggerOnHook() const;
    void TriggerOnTweak() const;
    void TriggerOnInit() const;
    void TriggerOnUpdate(float aDeltaTime) const;
    void TriggerOnDraw() const;
    void TriggerOnOverlayOpen() const;
    void TriggerOnOverlayClose() const;

    sol::object GetMod(const std::string& acName) const;
    void UnloadAllMods();
    void ReloadAllMods();
    bool ExecuteLua(const std::string& acCommand) const;
    void CollectGarbage() const;

    LuaSandbox& GetSandbox();
    LockedState GetLockedState() const noexcept;

    static size_t Size(CBaseRTTIType* apRttiType);
    static sol::object ToLua(LockedState& aState, CStackType& aResult);
    static CStackType ToRED(sol::object aObject, CBaseRTTIType* apRttiType, TiltedPhoques::Allocator* apAllocator);
    static void ToRED(sol::object aObject, CStackType& apType);

protected:
    void RegisterOverrides();

    sol::object Index(const std::string& acName, sol::this_state aState, sol::this_environment aEnv);
    sol::object GetSingletonHandle(const std::string& acName, sol::this_environment aThisEnv);

private:
    TiltedPhoques::Lockable<sol::state, std::recursive_mutex> m_lua;
    TiltedPhoques::Map<std::string, sol::object> m_properties{};
#if GAME_CYBERPUNK
    TiltedPhoques::Map<std::string, SingletonReference> m_singletons{};
#endif
    LuaSandbox m_sandbox;
    RTTIMapper m_mapper;
    ScriptStore m_store;
#if GAME_CYBERPUNK
    FunctionOverride m_override;
#endif
    const Paths& m_paths;
    D3D12& m_d3d12;
};
