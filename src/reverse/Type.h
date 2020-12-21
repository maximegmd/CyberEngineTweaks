#pragma once

#include <string>
#include <sol/sol.hpp>

namespace RED4ext {
	namespace REDreverse {
		struct CClass;
		struct CClassFunction;
	}
}

struct Type
{
    Type(sol::state_view aView, RED4ext::REDreverse::CClass* apClass, std::string aName);
	
    sol::object Index(const std::string& acName);
    sol::object NewIndex(const std::string& acName, sol::object aParam);
    sol::protected_function InternalIndex(const std::string& acName);
	
    sol::object Execute(RED4ext::REDreverse::CClassFunction* apFunc, const std::string& acName, sol::variadic_args args, sol::this_environment env, sol::this_state L, std::string& aReturnMessage);

private:
    sol::state_view m_lua;
    RED4ext::REDreverse::CClass* m_pType;
    std::string m_name;
    std::unordered_map<std::string, sol::object> m_properties;
};
