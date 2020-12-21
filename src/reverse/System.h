#pragma once

#include <string>
#include <sol/sol.hpp>

struct System
{
    System(sol::state_view aView, std::string aName);
	
    sol::object Index(const std::string& acName);
    sol::object NewIndex(const std::string& acName, sol::object aParam);
    sol::protected_function InternalIndex(const std::string& acName);
	
    sol::object Execute(const std::string& aFuncName, sol::variadic_args args, sol::this_environment env, sol::this_state L, std::string& aReturnMessage);

private:
    sol::state_view m_lua;
    std::string m_name;
    std::unordered_map<std::string, sol::object> m_properties;
};
