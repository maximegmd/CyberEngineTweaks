#include <stdafx.h>

#include "LuaVM.h"

#include <console/Console.h>

static std::unique_ptr<LuaVM> s_pLuaVM;

void LuaVM::Initialize()
{
    if (!s_pLuaVM)
    {
        s_pLuaVM.reset(new (std::nothrow) LuaVM);
        s_pLuaVM->Hook();
        s_pLuaVM->m_scripting.Initialize();
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

bool LuaVM::ExecuteLua(const std::string& acCommand)
{
    if (!m_initialized)
    {
        Console::Get().Log("Command not executed! LuaVM is not yet initialized!");
        return false;
    }

    return m_scripting.ExecuteLua(acCommand);
}

void LuaVM::Update(float aDeltaTime)
{
    if (!m_initialized && m_logCount.load(std::memory_order_relaxed) > 0)
        PostInitialize();

    if (!m_initialized)
        return;
    
    m_scripting.TriggerOnUpdate(aDeltaTime);
    m_scripting.TriggerOnDraw();
}

void LuaVM::ReloadAllMods()
{
    if (m_initialized)
    {
        m_scripting.ReloadAllMods();
        m_scripting.TriggerOnInit();
        spdlog::info("LuaVM::ReloadAllMods() finished!");
    }
}

void LuaVM::OnConsoleOpen()
{
    if (m_initialized)
        m_scripting.TriggerOnConsoleOpen();
}

void LuaVM::OnConsoleClose()
{
    if (m_initialized)
        m_scripting.TriggerOnConsoleClose();
}

bool LuaVM::IsInitialized() const
{
    return m_initialized;
}

void LuaVM::PostInitialize()
{
    assert(!m_initialized);
    m_scripting.TriggerOnInit();
    m_initialized = true;
    spdlog::info("LuaVM initialization complete!");
}
