#pragma once

#include "ScriptStore.h"
#include "reverse/SingletonReference.h"

struct D3D12;

struct Scripting
{
    Scripting(const Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12);
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

    static size_t Size(RED4ext::IRTTIType* apRttiType);
    static sol::object ToLua(sol::state_view aState, RED4ext::CStackType& aResult);
    static RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRttiType, TiltedPhoques::Allocator* apAllocator);
    static void ToRED(sol::object aObject, RED4ext::CStackType* apType);

protected:

    sol::object Index(const std::string& acName, sol::this_environment aThisEnv);
    sol::object NewIndex(const std::string& acName, sol::object aParam);
    sol::object GetSingletonHandle(const std::string& acName, sol::this_environment aThisEnv);
    sol::protected_function InternalIndex(const std::string& acName, sol::this_environment aThisEnv);
    
    sol::object Execute(const std::string& aFuncName, sol::variadic_args aArgs, sol::this_environment aThisEnv, sol::this_state aThisState, std::string& aReturnMessage) const;

    static void HandleOverridenFunction(RED4ext::IScriptable* aContext, RED4ext::CStackFrame* aFrame, int32_t* aOut,
                                        int64_t a4, struct Context* apCookie);

    void Override(const std::string& acTypeName, const std::string& acFullName, const std::string& acShortName,
                         bool aAbsolute, sol::protected_function aFunction, sol::this_environment aThisEnv);

private:
    sol::state m_lua{ };
    std::unordered_map<std::string, sol::object> m_properties{ };
    std::unordered_map<std::string, SingletonReference> m_singletons{ };
    LuaSandbox m_sandbox{ m_lua }; // some object must be passed here... will be reset in Initialize
    ScriptStore m_store;
    const Paths& m_paths;
    D3D12& m_d3d12;
};
