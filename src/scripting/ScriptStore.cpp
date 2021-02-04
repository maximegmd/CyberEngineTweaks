#include <stdafx.h>

#include "ScriptStore.h"

ScriptStore::ScriptStore(LuaSandbox& aLuaSandbox, const Paths& aPaths, VKBindings& aBindings)
    : m_sandbox(aLuaSandbox)
    , m_paths(aPaths)
    , m_bindings(aBindings)
{
}

void ScriptStore::LoadAll()
{
    m_vkBindInfos.clear();
    m_contexts.clear();
    m_sandbox.ResetState();

    auto consoleLogger = spdlog::get("scripting");

    const auto& cModsRoot = m_paths.ModsRoot();
    for (const auto& file : std::filesystem::directory_iterator(cModsRoot))
    {
        if (!file.is_directory())
            continue;

        auto fPath = file.path();
        auto fPathStr = fPath.string();
        if (!exists(fPath / "init.lua"))
        {
            consoleLogger->warn("Ignoring directory that does not contain init.lua! ('{}')", fPathStr);
            continue;
        }

        auto name = relative(fPath, cModsRoot).string();
        if (name.find('.') != std::string::npos)
        {
            consoleLogger->info("Ignoring directory containing '.', as this is reserved character! ('{}')", fPathStr);
            continue;
        }

        auto ctx = ScriptContext{m_sandbox, file.path(), name};
        if (ctx.IsValid())
        {
            auto& ctxBinds = ctx.GetBinds();
            m_vkBindInfos.insert(m_vkBindInfos.cend(), ctxBinds.cbegin(), ctxBinds.cend());
            m_contexts.emplace(name, std::move(ctx));
            consoleLogger->info("Mod {} loaded! ('{}')", name, fPathStr);
        }
        else
        {
            consoleLogger->error("Mod {} failed to load! ('{}')", name, fPathStr);
        }
    }

    m_bindings.InitializeMods(m_vkBindInfos);
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
