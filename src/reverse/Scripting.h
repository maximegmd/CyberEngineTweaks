#pragma once

#include <string>
#include <sol/sol.hpp>

#include "Type.h"

namespace RED4ext {
	namespace REDreverse {
		namespace Scripting {
			struct IScriptable;
		}
	}
}

struct Scripting
{
    Scripting();
	
    static Scripting& Get();
	
    bool ExecuteLua(const std::string& aCommand);
	
protected:

    sol::object Index(const std::string& acName);
    sol::object NewIndex(const std::string& acName, sol::object aParam);
    sol::object CreateSingletonHandle(const std::string& acName);
    sol::object CreateHandle(const std::string& acName, RED4ext::REDreverse::Scripting::IScriptable* apHandle);
    sol::protected_function InternalIndex(const std::string& acName);
	
	sol::object Execute(const std::string& aFuncName, sol::variadic_args args, sol::this_environment env, sol::this_state L, std::string& aReturnMessage);

private:
    sol::state m_lua;
    std::unordered_map<std::string, sol::object> m_properties;
    std::unordered_map<std::string, Type> m_systems;
};
