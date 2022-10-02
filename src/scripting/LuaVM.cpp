#include <stdafx.h>

#include "LuaVM.h"
#include "CET.h"
#include "overlay/Overlay.h"

std::optional<std::reference_wrapper<const VKBind>> LuaVM::GetBind(const VKModBind& acModBind) const
{
    return m_scripting.GetBind(acModBind);
}

std::optional<std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>> LuaVM::GetBinds(const std::string& acModName) const
{
    return m_scripting.GetBinds(acModName);
}

const TiltedPhoques::Map<std::string, std::reference_wrapper<const TiltedPhoques::Vector<VKBind>>>& LuaVM::GetAllBinds() const
{
    return m_scripting.GetAllBinds();
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
    if (!m_initialized)
    {
        if (m_logCount.load(std::memory_order_relaxed) > 0)
            PostInitializeMods();

        return;
    }

    m_scripting.TriggerOnUpdate(aDeltaTime);
}

void LuaVM::Draw()
{
    if (!m_initialized || m_drawBlocked)
        return;

    m_scripting.TriggerOnDraw();
}

void LuaVM::ReloadAllMods()
{
    if (m_initialized)
    {
        m_scripting.ReloadAllMods();
        m_scripting.TriggerOnTweak();
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

void LuaVM::BlockDraw(bool aBlockDraw)
{
    m_drawBlocked = aBlockDraw;
}

void LuaVM::RemoveTDBIDDerivedFrom(uint64_t aDBID)
{
    std::lock_guard<std::shared_mutex> _{ m_tdbidLock };

    const auto it = m_tdbidDerivedLookup.find(aDBID);
    if (it != m_tdbidDerivedLookup.end())
    {
        for (uint64_t flatID : it->second)
        {
            m_tdbidLookup.erase(flatID);
        }

        m_tdbidDerivedLookup.erase(it);
    }
}

bool LuaVM::GetTDBIDDerivedFrom(uint64_t aDBID, TiltedPhoques::Vector<uint64_t>& aDerivedList)
{
    std::shared_lock<std::shared_mutex> _{ m_tdbidLock };

    const auto it = m_tdbidDerivedLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidDerivedLookup.end())
        return false;

    aDerivedList.reserve(it->second.size());
    std::copy(it->second.begin(), it->second.end(), std::back_inserter(aDerivedList));
    return true;
}

uint64_t LuaVM::GetTDBIDBase(uint64_t aDBID)
{
    std::shared_lock<std::shared_mutex> _{ m_tdbidLock };

    const auto it = m_tdbidLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidLookup.end())
        return 0;
    return it->second.base;
}

TDBIDLookupEntry LuaVM::GetTDBIDLookupEntry(uint64_t aDBID)
{
    std::shared_lock<std::shared_mutex> _{ m_tdbidLock };

    const auto it = m_tdbidLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidLookup.end())
        return { 0, "<unknown>" };

    return it->second;
}

std::string LuaVM::GetTDBDIDDebugString(TDBID aDBID)
{
    RED4ext::TweakDBID internal(aDBID.value);
    return internal.HasTDBOffset()
        ? fmt::format("<TDBID:{:08X}:{:02X}:{:06X}>", internal.name.hash, internal.name.length, internal.ToTDBOffset())
        : fmt::format("<TDBID:{:08X}:{:02X}>", internal.name.hash, internal.name.length);
}

std::string LuaVM::GetTDBIDString(uint64_t aDBID)
{
    std::shared_lock<std::shared_mutex> _{ m_tdbidLock };

    auto it = m_tdbidLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidLookup.end())
        return GetTDBDIDDebugString(TDBID{ aDBID });

    std::string string = it->second.name;
    uint64_t base = it->second.base;
    while (base != 0)
    {
        it = m_tdbidLookup.find(it->second.base);
        if (it == m_tdbidLookup.end())
        {
            string.insert(0, GetTDBDIDDebugString(TDBID{ base }));
            break;
        }
        string.insert(0, it->second.name);
        base = it->second.base;
    }

    return string;
}

void LuaVM::RegisterTDBIDString(uint64_t aValue, uint64_t aBase, const std::string& aName)
{
    if (aValue == 0) return;
    std::lock_guard<std::shared_mutex> _{ m_tdbidLock };

    m_tdbidLookup[aValue] = { aBase, aName };
    if (aBase != 0)
        m_tdbidDerivedLookup[aBase].insert(aValue);
}

void LuaVM::PostInitializeScripting()
{
    m_scripting.PostInitializeScripting();
}

void LuaVM::PostInitializeTweakDB()
{
    m_scripting.TriggerOnTweak();
}

void LuaVM::PostInitializeMods()
{
    assert(!m_initialized);

    m_scripting.PostInitializeMods();
    m_scripting.TriggerOnInit();
    if (CET::Get().GetOverlay().IsEnabled())
        m_scripting.TriggerOnOverlayOpen();

    spdlog::get("scripting")->info("LuaVM: initialization finished!");

    m_initialized = true;
}
