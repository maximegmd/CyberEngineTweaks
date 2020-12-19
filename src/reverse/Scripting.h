#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "REDString.h"

struct Scripting
{
	static bool Execute(const std::string& aCommand, std::string& aReturnMessage);
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
