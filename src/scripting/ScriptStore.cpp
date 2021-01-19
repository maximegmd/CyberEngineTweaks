#include <stdafx.h>

#include "ScriptStore.h"

void ScriptStore::LoadAll(sol::state_view aStateView)
{
    m_vkBindInfos.clear();
    m_contexts.clear();

    const auto& cModsRoot = Paths::Get().ModsRoot();
    for (const auto& file : std::filesystem::directory_iterator(cModsRoot))
    {
        if (!file.is_directory())
            continue;

        auto fPath = file.path();
        auto fPathStr = fPath.string();
        if (!exists(fPath / "init.lua"))
        {
            Logger::ToConsoleFmt("Ignoring directory which misses init.lua! ('{}')", fPathStr);
            Logger::WarningToModsFmt("Ignoring directory which misses init.lua! ('{}')", fPathStr);
            continue;
        }

        auto name = relative(fPath, cModsRoot).string();
        if (name.find('.') != std::string::npos)
        {
            Logger::ToConsoleFmt("Ignoring directory with '.', as this is reserved character! ('{}')", fPathStr);
            Logger::WarningToModsFmt("Ignoring directory with '.', as this is reserved character! ('{}')", fPathStr);
            continue;
        }
        
        auto ctx = ScriptContext{ aStateView, file.path() };
        if (ctx.IsValid())
        {
            auto& ctxBinds = ctx.GetBinds();
            m_vkBindInfos.insert(m_vkBindInfos.cend(), ctxBinds.cbegin(), ctxBinds.cend());
            m_contexts.emplace(name, std::move(ctx));
            Logger::ToConsoleFmt("Mod {} loaded! ('{}')", name, fPathStr);
            Logger::InfoToModsFmt("Mod {} loaded! ('{}')", name, fPathStr);
        }
        else
        {
            Logger::ToConsoleFmt("Mod {} failed loaded! ('{}')", name, fPathStr);
            Logger::ErrorToModsFmt("Mod {} failed loaded! ('{}')", name, fPathStr);
        }
    }

    VKBindings::InitializeMods(m_vkBindInfos);
}

std::vector<VKBindInfo>& ScriptStore::GetBinds()
{
    return m_vkBindInfos;
}

void ScriptStore::TriggerOnInit() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnInit();
}

void ScriptStore::TriggerOnUpdate(float aDeltaTime) const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnUpdate(aDeltaTime);
}

void ScriptStore::TriggerOnDraw() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnDraw();
}

void ScriptStore::TriggerOnOverlayOpen() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnOverlayOpen();
}

void ScriptStore::TriggerOnOverlayClose() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnOverlayClose();
}

sol::object ScriptStore::GetMod(const std::string& acName) const
{
    const auto itor = m_contexts.find(acName);
    if (itor != std::end(m_contexts))
        return itor->second.GetRootObject();

    return sol::nil;
}
