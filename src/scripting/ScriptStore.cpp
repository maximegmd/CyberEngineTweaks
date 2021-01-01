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

    for (auto& file : std::filesystem::directory_iterator(cScriptsPath))
    {
        if (!file.is_directory())
            continue;

        if (!exists(file.path() / "init.lua"))
            continue;

        auto name = relative(file.path(), cScriptsPath).string();

        auto ctx = ScriptContext{ aStateView, file.path() };
    }
}
