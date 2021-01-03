#include <stdafx.h>

#include "LuaVM.h"

#include <Image.h>
#include <Options.h>
#include <scripting/Scripting.h>

#include "console/Console.h"

static std::unique_ptr<LuaVM> s_pLuaVM;

void LuaVM::Initialize()
{
    if (!s_pLuaVM)
    {
        s_pLuaVM.reset(new (std::nothrow) LuaVM);
        s_pLuaVM->Hook();
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

bool LuaVM::ExecuteLua(const std::string& command)
{
    if (!m_initialized)
    {
        if (Options::Get().Console)
            Console::Get().Log("Command not executed! LuaVM is not yet initialized!");
    }

    return Scripting::Get().ExecuteLua(command);
}

void LuaVM::PostInitialize()
{
    if (m_initialized)
        return;

    Scripting::Get().GetStore().TriggerOnInit();
    spdlog::info("LuaVM initialization complete!");
    m_initialized = true;
}

LuaVM::LuaVM() = default;

LuaVM::~LuaVM() = default;
