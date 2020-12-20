#pragma once

#include <string>
#include <sol/sol.hpp>

struct Scripting
{
    Scripting();
	
    static Scripting& Get();
	
    bool ExecuteLua(const std::string& aCommand);
	
protected:

    static sol::protected_function Index(Scripting& aThis, const std::string& acName);
    sol::protected_function InternalIndex(const std::string& acName);
	
	bool Execute(const std::string& aFuncName, sol::variadic_args args, sol::this_environment env, sol::this_state L, std::string& aReturnMessage);

private:
    sol::state m_lua;
};
