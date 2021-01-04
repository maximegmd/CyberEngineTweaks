#include <stdafx.h>

#include "LuaVM.h"

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

void LuaVM::Update(float aDeltaTime)
{
    if (!m_initialized && m_logCount.load(std::memory_order_relaxed) > 0)
        PostInitialize();

    if (!m_initialized)
        return;

    Scripting::Get().GetStore().TriggerOnUpdate(aDeltaTime);
}

bool LuaVM::ExecuteLua(const std::string& aCommand)
{
    if (!m_initialized)
    {
        Console::Get().Log("Command not executed! LuaVM is not yet initialized!");
        return false;
    }

    return Scripting::Get().ExecuteLua(aCommand);
}

void LuaVM::PostInitialize()
{
    assert(!m_initialized);
    Scripting::Get().GetStore().TriggerOnInit();
    m_initialized = true;
    spdlog::info("LuaVM initialization complete!");
}

LuaVM::LuaVM() = default;

LuaVM::~LuaVM() = default;
