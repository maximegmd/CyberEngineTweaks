#include <stdafx.h>

#include "ScriptStore.h"

#include "Options.h"

ScriptStore::ScriptStore()
{
    
}

ScriptStore::~ScriptStore()
{

}

void ScriptStore::LoadAll(sol::state_view aStateView)
{
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
            spdlog::warn("Mod {} failed to load!", file.path().string());
    }
}

void ScriptStore::TriggerOnUpdate(float deltaTime) const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnUpdate(deltaTime);
}

void ScriptStore::TriggerOnInit() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnInit();
}

sol::object ScriptStore::Get(const std::string& acName) const
{
    const auto itor = m_contexts.find(acName);
    if (itor != std::end(m_contexts))
        return itor->second.GetObject();

    return sol::nil;
}
