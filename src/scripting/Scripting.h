#pragma once

#include "FunctionOverride.h"
#include "ScriptStore.h"
#include "reverse/RTTIMapper.h"
#include "reverse/SingletonReference.h"

struct D3D12;

struct Scripting
{
    using LockedState = TiltedPhoques::Locked<sol::state, std::recursive_mutex>;

    Scripting(const Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12, Options& aOptions);
    ~Scripting() = default;

    void Initialize();
    void PostInitializeScripting();
    void PostInitializeMods();

    [[nodiscard]] std::optional<std::reference_wrapper<const VKBind>> GetBind(const VKModBind& acModBind) const;
    [[nodiscard]] std::optional<std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>> GetBinds(const std::string& acModName) const;
    [[nodiscard]] const TiltedPhoques::Map<std::string, std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>>& GetAllBinds() const;

    void TriggerOnTweak() const;
    void TriggerOnInit() const;
    void TriggerOnUpdate(float aDeltaTime) const;
    void TriggerOnDraw() const;
    void TriggerOnOverlayOpen() const;
    void TriggerOnOverlayClose() const;

    sol::object GetMod(const std::string& acName) const;
    void ReloadAllMods();
    bool ExecuteLua(const std::string& acCommand);
    void CollectGarbage();

    LockedState GetState() const noexcept;
    std::string GetGlobalName() const noexcept;

    static size_t Size(RED4ext::CBaseRTTIType* apRttiType);
    static sol::object ToLua(LockedState& aState, RED4ext::CStackType& aResult);
    static RED4ext::CStackType ToRED(sol::object aObject, RED4ext::CBaseRTTIType* apRttiType,
                                     TiltedPhoques::Allocator* apAllocator);
    static void ToRED(sol::object aObject, RED4ext::CStackType& apType);

protected:
    void RegisterOverrides();

    sol::object Index(const std::string& acName, sol::this_state aState, sol::this_environment aEnv);
    sol::object GetSingletonHandle(const std::string& acName, sol::this_environment aThisEnv);

private:
    TiltedPhoques::Lockable<sol::state, std::recursive_mutex> m_lua;
    TiltedPhoques::Map<std::string, sol::object> m_properties{ };
    TiltedPhoques::Map<std::string, SingletonReference> m_singletons{ };
    std::string m_global{ "Global" };
    RTTIMapper m_mapper;
    LuaSandbox m_sandbox;
    ScriptStore m_store;
    FunctionOverride m_override;
    const Paths& m_paths;
    D3D12& m_d3d12;
    mutable std::recursive_mutex m_vmLock;
};
