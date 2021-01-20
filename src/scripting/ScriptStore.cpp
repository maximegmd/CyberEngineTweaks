#include <stdafx.h>

#include "ScriptStore.h"

void ScriptStore::LoadAll(sol::state_view aStateView)
{
    m_vkBindInfos.clear();
    m_contexts.clear();

    auto consoleLogger = spdlog::get("console");

    const auto& cModsRoot = Paths::Get().ModsRoot();
    for (const auto& file : std::filesystem::directory_iterator(cModsRoot))
    {
        if (!file.is_directory())
            continue;

        auto fPath = file.path();
        auto fPathStr = fPath.string();
        if (!exists(fPath / "init.lua"))
        {
            consoleLogger->info("Ignoring directory which misses init.lua! ('{}')", fPathStr);
            spdlog::warn("Ignoring directory which misses init.lua! ('{}')", fPathStr);
            continue;
        }

        auto name = relative(fPath, cModsRoot).string();
        if (name.find('.') != std::string::npos)
        {
            consoleLogger->info("Ignoring directory with '.', as this is reserved character! ('{}')", fPathStr);
            spdlog::warn("Ignoring directory with '.', as this is reserved character! ('{}')", fPathStr);
            continue;
        }
        
        auto ctx = ScriptContext{ aStateView, file.path() };
        if (ctx.IsValid())
        {
            auto& ctxBinds = ctx.GetBinds();
            m_vkBindInfos.insert(m_vkBindInfos.cend(), ctxBinds.cbegin(), ctxBinds.cend());
            m_contexts.emplace(name, std::move(ctx));
            consoleLogger->info("Mod {} loaded! ('{}')", name, fPathStr);
            spdlog::info("Mod {} loaded! ('{}')", name, fPathStr);
        }
        else
        {
            consoleLogger->info("Mod {} failed loaded! ('{}')", name, fPathStr);
            spdlog::error("Mod {} failed loaded! ('{}')", name, fPathStr);
        }
    }

    VKBindings::InitializeMods(m_vkBindInfos);
}

const std::vector<VKBindInfo>& ScriptStore::GetBinds() const
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
