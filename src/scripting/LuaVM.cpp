#include <stdafx.h>

#include "LuaVM.h"

#include <Image.h>
#include <Options.h>
#include <scripting/Scripting.h>

#include "console/Console.h"

static std::unique_ptr<LuaVM> s_pLuaVM;

void LuaVM::Initialize(Image* apImage)
{
    if (!s_pLuaVM)
    {
        s_pLuaVM.reset(new (std::nothrow) LuaVM);
        s_pLuaVM->Hook(apImage);
    }
}

void LuaVM::Shutdown()
{
    s_pLuaVM = nullptr;
}

LuaVM& LuaVM::Get()
{
    return *s_pLuaVM;
}

void LuaVM::Update(float deltaTime)
{
    if (!m_initialized)
        return;

    Scripting::Get().GetStore().TriggerOnUpdate(deltaTime);
}

bool LuaVM::ExecuteLua(const std::string& aCommand)
{
    if (!m_initialized)
    {
        if (Options::Get().Console)
            Console::Get().Log("Command not executed! LuaVM is not yet initialized!");
    }

    return Scripting::Get().ExecuteLua(aCommand);
}

LuaVM::LuaVM() = default;

LuaVM::~LuaVM() = default;
