#include <stdafx.h>

#include "ScriptStore.h"

void ScriptStore::LoadAll(sol::state_view aStateView)
{
    m_contexts.clear();

    const auto cScriptsPath = Paths::ScriptsPath;

    for (const auto& file : std::filesystem::directory_iterator(cScriptsPath))
    {
        if (!file.is_directory())
            continue;

        if (!exists(file.path() / "init.lua"))
            continue;

        auto name = relative(file.path(), cScriptsPath).string();

        auto ctx = ScriptContext{ aStateView, file.path() };
        auto fpathString = file.path().string();
        if (ctx.IsValid())
        {
            Logger::ToConsoleFmt("Mod {} loaded!", fpathString);
            Logger::InfoToModsFmt("Mod {} loaded!", fpathString);
            m_contexts.emplace(name, std::move(ctx));
        }
        else
        {
            Logger::ToConsoleFmt("Mod {} failed loaded!", fpathString);
            Logger::ErrorToModsFmt("Mod {} failed loaded!", fpathString);
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

void ScriptStore::TriggerOnToolbarOpen() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnToolbarOpen();
}

void ScriptStore::TriggerOnToolbarClose() const
{
    for (const auto& kvp : m_contexts)
        kvp.second.TriggerOnToolbarClose();
}

sol::object ScriptStore::GetMod(const std::string& acName) const
{
    const auto itor = m_contexts.find(acName);
    if (itor != std::end(m_contexts))
        return itor->second.GetRootObject();

    return sol::nil;
}
