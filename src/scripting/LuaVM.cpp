#include <stdafx.h>

#include "LuaVM.h"

#include "overlay/Overlay.h"

static std::unique_ptr<LuaVM> s_pLuaVM;

void LuaVM::Initialize()
{
    if (!s_pLuaVM)
    {
        s_pLuaVM.reset(new (std::nothrow) LuaVM);
        s_pLuaVM->Hook();
        s_pLuaVM->m_scripting.Initialize();
        s_pLuaVM->m_initialized.fetch_add(1);
    }
}

void LuaVM::Shutdown()
{
    s_pLuaVM = nullptr;
}

LuaVM& LuaVM::Get()
{
    assert(s_pLuaVM);
    return *s_pLuaVM;
}

const std::vector<VKBindInfo>& LuaVM::GetBinds() const
{
    return m_scripting.GetBinds();
}

bool LuaVM::ExecuteLua(const std::string& acCommand)
{
    if (!IsInitialized())
    {
        spdlog::get("console")->info("Command not executed! LuaVM is not yet initialized!");
        return false;
    }

    return m_scripting.ExecuteLua(acCommand);
}

void LuaVM::Update(float aDeltaTime)
{
    if (m_logCount.load(std::memory_order_relaxed) > 0)
        PostInitialize();

    if (!IsInitialized())
        return;
    
    m_scripting.TriggerOnUpdate(aDeltaTime);
    m_scripting.TriggerOnDraw();
}

void LuaVM::ReloadAllMods()
{
    if (IsInitialized())
    {
        m_scripting.ReloadAllMods();
        m_scripting.TriggerOnInit();
        if (Overlay::Get().IsEnabled())
            m_scripting.TriggerOnOverlayOpen();

        spdlog::info("LuaVM: Reloaded all mods!");
        spdlog::get("console")->info("LuaVM: Reloaded all mods!");
    }
}

void LuaVM::OnOverlayOpen()
{
    if (IsInitialized())
        m_scripting.TriggerOnOverlayOpen();
}

void LuaVM::OnOverlayClose()
{
    if (IsInitialized())
        m_scripting.TriggerOnOverlayClose();
}

bool LuaVM::IsInitialized() const
{
    return m_initialized.load(std::memory_order_relaxed) > 1;
}

void LuaVM::PostInitialize()
{
    if (m_initialized.load(std::memory_order_relaxed) != 1)
        return

    m_scripting.TriggerOnInit();
    if (Overlay::Get().IsEnabled())
        m_scripting.TriggerOnOverlayOpen();

    spdlog::info("LuaVM: initialization finished!");
    spdlog::get("console")->info("LuaVM: initialization finished!");

    m_initialized.fetch_add(1);;
}
