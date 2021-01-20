#include <stdafx.h>

#include "LuaVM.h"
#include "CET.h"
#include "overlay/Overlay.h"

const std::vector<VKBindInfo>& LuaVM::GetBinds() const
{
    return m_scripting.GetBinds();
}

bool LuaVM::ExecuteLua(const std::string& acCommand)
{
    if (!m_initialized)
    {
        spdlog::get("scripting")->info("Command not executed! LuaVM is not yet initialized!");
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

        if (CET::Get().GetOverlay().IsEnabled())
            m_scripting.TriggerOnOverlayOpen();

        spdlog::get("scripting")->info("LuaVM: Reloaded all mods!");
    }
}

void LuaVM::OnOverlayOpen() const
{
    if (m_initialized)
        m_scripting.TriggerOnOverlayOpen();
}

void LuaVM::OnOverlayClose() const
{
    if (m_initialized)
        m_scripting.TriggerOnOverlayClose();
}

void LuaVM::Initialize()
{
    if (!IsInitialized())
    {
        m_scripting.Initialize();
    }
}

bool LuaVM::IsInitialized() const
{
    return m_initialized;
}

void LuaVM::PostInitialize()
{
    assert(!m_initialized);

    m_scripting.TriggerOnInit();
    if (CET::Get().GetOverlay().IsEnabled())
        m_scripting.TriggerOnOverlayOpen();

    spdlog::get("scripting")->info("LuaVM: initialization finished!");

    m_initialized = true;
}
