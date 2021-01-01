#pragma once

#include "ScriptStore.h"
#include "reverse/SingletonReference.h"

struct Scripting
{
    Scripting();
	
    static Scripting& Get();
	
    bool ExecuteLua(const std::string& aCommand);

	static size_t Size(RED4ext::IRTTIType* apRtti);
	static sol::object ToLua(sol::state_view aState, RED4ext::CStackType& aResult);
	static RED4ext::CStackType ToRED(sol::object aObject, RED4ext::IRTTIType* apRtti, TiltedPhoques::Allocator* apAllocator);

protected:

	void Initialize();

    sol::object Index(const std::string& acName);
    sol::object NewIndex(const std::string& acName, sol::object aParam);
    sol::object GetSingletonHandle(const std::string& acName);
    sol::object CreateHandle(const std::string& acName, RED4ext::IScriptable* apHandle);
    sol::protected_function InternalIndex(const std::string& acName);
	
	sol::object Execute(const std::string& aFuncName, sol::variadic_args args, sol::this_environment env, sol::this_state L, std::string& aReturnMessage);

private:
    sol::state m_lua;
    std::unordered_map<std::string, sol::object> m_properties;
    std::unordered_map<std::string, SingletonReference> m_singletons;
    ScriptStore m_store;
};
