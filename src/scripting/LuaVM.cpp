#include <stdafx.h>

#include "LuaVM.h"

#include "EngineTweaks.h"

const VKBind* LuaVM::GetBind(const VKModBind& acModBind) const
{
    return m_scripting.GetBind(acModBind);
}

const TiltedPhoques::Vector<VKBind>* LuaVM::GetBinds(const std::string& acModName) const
{
    return m_scripting.GetBinds(acModName);
}

const TiltedPhoques::Map<std::string, std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>>& LuaVM::GetAllBinds() const
{
    return m_scripting.GetAllBinds();
}

bool LuaVM::ExecuteLua(const std::string& acCommand) const
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
    if (!m_initialized)
        return;

    EngineTweaks::Get().GetBindings().Update();

    if (m_reload)
    {
        m_scripting.ReloadAllMods();

        m_reload = false;
    }

    m_scripting.TriggerOnUpdate(aDeltaTime);
}

void LuaVM::Draw() const
{
    if (!m_initialized || m_drawBlocked)
        return;

    m_scripting.TriggerOnDraw();
}

void LuaVM::ReloadAllMods()
{
    if (m_initialized)
        m_reload = true;
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
        m_scripting.Initialize();
}

bool LuaVM::IsInitialized() const
{
    return m_initialized;
}

void LuaVM::BlockDraw(bool aBlockDraw)
{
    m_drawBlocked = aBlockDraw;
}

void LuaVM::PostInitializeScripting()
{
    m_scripting.PostInitializeScripting();
}

void LuaVM::PostInitializeMods()
{
    assert(!m_initialized);

    m_scripting.PostInitializeMods();

    spdlog::get("scripting")->info("LuaVM: initialization finished!");

    m_initialized = true;
}
