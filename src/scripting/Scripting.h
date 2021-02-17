#pragma once

#include "FunctionOverride.h"
#include "ScriptStore.h"
#include "reverse/SingletonReference.h"

struct D3D12;

struct Scripting
{
    using LockedState = TiltedPhoques::Locked<sol::state, std::recursive_mutex>;

    Scripting(const Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12, Options& aOptions);
    ~Scripting() = default;

    void Initialize();

    const std::vector<VKBindInfo>& GetBinds() const;

    void TriggerOnInit() const;
    void TriggerOnUpdate(float aDeltaTime) const;
    void TriggerOnDraw() const;
    
    void TriggerOnOverlayOpen() const;
    void TriggerOnOverlayClose() const;

    sol::object GetMod(const std::string& acName) const;
    void ReloadAllMods();

    bool ExecuteLua(const std::string& acCommand);
    LockedState GetState() const noexcept;

    static size_t Size(RED4ext::IRTTIType* apRttiType);
    static sol::object ToLua(LockedState& aState, RED4ext::CStackType& aResult);
    static RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRttiType, TiltedPhoques::Allocator* apAllocator);
    static void ToRED(sol::object aObject, RED4ext::CStackType* apType);

protected:

    sol::object Index(const std::string& acName, sol::this_environment aThisEnv);
    sol::object NewIndex(const std::string& acName, sol::object aParam);
    sol::object GetSingletonHandle(const std::string& acName, sol::this_environment aThisEnv);
    sol::protected_function InternalIndex(const std::string& acName, sol::this_environment aThisEnv);
    
    sol::object Execute(const std::string& aFuncName, sol::variadic_args aArgs, std::string& aReturnMessage, sol::this_state aState) const;

private:
    TiltedPhoques::Lockable<sol::state, std::recursive_mutex> m_lua;
    std::unordered_map<std::string, sol::object> m_properties{ };
    std::unordered_map<std::string, SingletonReference> m_singletons{ };
    LuaSandbox m_sandbox{ this }; // some object must be passed here... will be reset in Initialize
    ScriptStore m_store;
    FunctionOverride m_override;
    const Paths& m_paths;
    D3D12& m_d3d12;
    mutable std::recursive_mutex m_vmLock;
};
