#include <stdafx.h>

#include "ScriptStore.h"

#include "Options.h"

void ScriptStore::LoadAll(sol::state_view aStateView)
{
    m_contexts.clear();

    const auto cScriptsPath = Options::Get().ScriptsPath;

    for (const auto& file : std::filesystem::directory_iterator(cScriptsPath))
    {
        if (!file.is_directory())
            continue;

        if (!exists(file.path() / "init.lua"))
            continue;

        auto name = relative(file.path(), cScriptsPath).string();

        auto ctx = ScriptContext{ aStateView, file.path() };
        if (ctx.IsValid())
        {
            spdlog::info("Mod {} loaded!", file.path().string());
            m_contexts.emplace(name, std::move(ctx));
        }
        else
        {
            spdlog::warn("Mod {} failed to load!", file.path().string());
        }
    }
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

void ScriptStore::TriggerOnConsoleOpen() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnConsoleOpen();
}

void ScriptStore::TriggerOnConsoleClose() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnConsoleClose();
}

sol::object ScriptStore::GetMod(const std::string& acName) const
{
    const auto itor = m_contexts.find(acName);
    if (itor != std::end(m_contexts))
        return itor->second.GetRootObject();

    return sol::nil;
}
