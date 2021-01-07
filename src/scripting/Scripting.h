#pragma once

#include "ScriptStore.h"
#include "reverse/SingletonReference.h"

struct Scripting
{
    Scripting() = default;
    ~Scripting() = default;

    void Initialize();

    void TriggerOnInit() const;
    void TriggerOnUpdate(float aDeltaTime) const;
    void TriggerOnDraw() const;
    
    void TriggerOnConsoleOpen() const;
    void TriggerOnConsoleClose() const;

    sol::object GetMod(const std::string& acName) const;
    void ReloadAllMods();
    
    bool ExecuteLua(const std::string& acCommand);

    static size_t Size(RED4ext::IRTTIType* apRtti);
    static sol::object ToLua(sol::state_view aState, RED4ext::CStackType& aResult);
    static RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator);

protected:

    sol::object Index(const std::string& acName);
    sol::object NewIndex(const std::string& acName, sol::object aParam);
    sol::object GetSingletonHandle(const std::string& acName);
    sol::protected_function InternalIndex(const std::string& acName);
    
    sol::object Execute(const std::string& aFuncName, sol::variadic_args args, sol::this_environment env, sol::this_state L, std::string& aReturnMessage) const;

private:
    sol::state m_lua{ };
    std::unordered_map<std::string, sol::object> m_properties{ };
    std::unordered_map<std::string, SingletonReference> m_singletons{ };
    ScriptStore m_store{ };
};
