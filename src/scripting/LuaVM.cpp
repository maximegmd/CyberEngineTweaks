#include "stdafx.h"

#include "LuaVM.h"

#include <CET.h>

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

    CET::Get().GetBindings().Update();

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

void LuaVM::RemoveTDBIDDerivedFrom(uint64_t aDBID)
{
    std::lock_guard _{m_tdbidLock};

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
    std::lock_guard _{m_tdbidLock};

    const auto it = m_tdbidDerivedLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidDerivedLookup.end())
        return false;

    aDerivedList.reserve(it->second.size());
    std::ranges::copy(it->second, std::back_inserter(aDerivedList));
    return true;
}

uint64_t LuaVM::GetTDBIDBase(uint64_t aDBID)
{
    std::lock_guard _{m_tdbidLock};

    const auto it = m_tdbidLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidLookup.end())
        return 0;
    return it->second.base;
}

TDBIDLookupEntry LuaVM::GetTDBIDLookupEntry(uint64_t aDBID)
{
    std::lock_guard _{m_tdbidLock};

    const auto it = m_tdbidLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidLookup.end())
        return {0, "<unknown>"};

    return it->second;
}

std::string LuaVM::GetTDBDIDDebugString(TDBID aDBID) const
{
    RED4ext::TweakDBID internal(aDBID.value);
    return internal.HasTDBOffset() ? fmt::format("<TDBID:{:08X}:{:02X}:{:06X}>", internal.name.hash, internal.name.length, internal.ToTDBOffset())
                                   : fmt::format("<TDBID:{:08X}:{:02X}>", internal.name.hash, internal.name.length);
}

std::string LuaVM::GetTDBIDString(uint64_t aDBID, bool aOnlyRegistered)
{
    std::lock_guard _{m_tdbidLock};

    auto it = m_tdbidLookup.find(aDBID & 0xFFFFFFFFFF);
    if (it == m_tdbidLookup.end())
        return aOnlyRegistered ? "" : GetTDBDIDDebugString(TDBID{aDBID});

    std::string string = it->second.name;
    uint64_t base = it->second.base;
    while (base != 0)
    {
        it = m_tdbidLookup.find(it->second.base);
        if (it == m_tdbidLookup.end())
        {
            string.insert(0, GetTDBDIDDebugString(TDBID{base}));
            break;
        }
        string.insert(0, it->second.name);
        base = it->second.base;
    }

    return string;
}

void LuaVM::RegisterTDBIDString(uint64_t aValue, uint64_t aBase, const std::string& acString)
{
    if (aValue == 0)
        return;
    std::lock_guard _{m_tdbidLock};

    m_tdbidLookup[aValue] = {aBase, acString};
    if (aBase != 0)
        m_tdbidDerivedLookup[aBase].insert(aValue);
}

void LuaVM::PostInitializeScripting()
{
    m_scripting.PostInitializeScripting();
}

void LuaVM::PostInitializeTweakDB()
{
    m_scripting.PostInitializeTweakDB();
}

void LuaVM::PostInitializeMods()
{
    assert(!m_initialized);

    m_scripting.PostInitializeMods();

    spdlog::get("scripting")->info("LuaVM: initialization finished!");

    m_initialized = true;
}
