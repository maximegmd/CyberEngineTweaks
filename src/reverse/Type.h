#pragma once

#include <string>
#include <sol/sol.hpp>

namespace RED4ext {
	namespace REDreverse {
		namespace Scripting {
			struct IScriptable;
		}

		struct CClass;
		struct CClassFunction;
	}
}

struct Type
{
    Type(sol::state_view aView, RED4ext::REDreverse::CClass* apClass);

    sol::object Index(const std::string& acName);
    sol::object NewIndex(const std::string& acName, sol::object aParam);
    sol::protected_function InternalIndex(const std::string& acName);
	std::string GetName() const;
	
    sol::object Execute(RED4ext::REDreverse::CClassFunction* apFunc, const std::string& acName, sol::variadic_args args, sol::this_environment env, sol::this_state L, std::string& aReturnMessage);

protected:

	virtual RED4ext::REDreverse::Scripting::IScriptable* GetHandle() = 0;

	RED4ext::REDreverse::CClass* m_pType{ nullptr };
	
private:
    sol::state_view m_lua;
    std::unordered_map<std::string, sol::object> m_properties;
};
