#pragma once

#include <string>
#include <sol/sol.hpp>

#include "System.h"

struct Scripting
{
    Scripting();
	
    static Scripting& Get();
	
    bool ExecuteLua(const std::string& aCommand);
	
protected:

    sol::object Index(const std::string& acName);
    sol::object NewIndex(const std::string& acName, sol::object aParam);
    sol::object GetSystem(const std::string& acName);
    sol::protected_function InternalIndex(const std::string& acName);
	
	bool Execute(const std::string& aFuncName, sol::variadic_args args, sol::this_environment env, sol::this_state L, std::string& aReturnMessage);

private:
    sol::state m_lua;
    std::unordered_map<std::string, sol::object> m_properties;
    std::unordered_map<std::string, System> m_systems;
};
