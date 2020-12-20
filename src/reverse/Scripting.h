#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <sol/sol.hpp>

#include "REDString.h"

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

    uintptr_t arg0Rtti{ 0 };
    uintptr_t argiRtti{ 0 };
    uintptr_t ctorOffset{ 0 };
    uintptr_t execOffset{ 0 };
    uintptr_t arg0RttiPtr{ 0 };
    uintptr_t argiRttiPtr{ 0 };
    void* m_pCtor{ nullptr };
    void* m_pExec{ nullptr };
};

struct Result
{
    Result();
	
    REDString someStr;
    uint64_t code{ 7 };
    REDString* output;
    int number2{ 0 };
    uint8_t pad[0x1000]; // pad just in case
};

static_assert(offsetof(Result, code) == 0x20);
static_assert(offsetof(Result, output) == 0x28);

struct ScriptArgs
{
    REDString* args;
    uint32_t pad8;
    uint32_t argCount;
    uint8_t pad[0x100]; // pad just in case
};
