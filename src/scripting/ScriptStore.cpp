#include <stdafx.h>

#include "ScriptStore.h"

#include "Options.h"

void ScriptStore::LoadAll()
{
    const auto cScriptsPath = Options::Get().Path / "autorun_scripts";

    for (auto& file : std::filesystem::directory_iterator(cScriptsPath))
    {
        if (!file.is_directory() && file.path().extension() == ".lua")
        {
            const auto modName = file.path().filename();
        }
    }
}
