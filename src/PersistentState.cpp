#include <stdafx.h>

#include "Paths.h"
#include "Utils.h"

void OverlayPersistentState::Load(const nlohmann::json& aConfig)
{
    ConsoleToggled = aConfig.value("console_toggled", ConsoleToggled);
    BindingsToggled = aConfig.value("bindings_toggled", BindingsToggled);
    SettingsToggled = aConfig.value("settings_toggled", SettingsToggled);
    TweakDBEditorToggled = aConfig.value("tweakdbeditor_toggled", TweakDBEditorToggled);
    GameLogToggled = aConfig.value("gamelog_toggled", GameLogToggled);
    ImGuiDebugToggled = aConfig.value("imguidebug_toggled", ImGuiDebugToggled);
}

nlohmann::json OverlayPersistentState::Save() const
{
    return {{"console_toggled", ConsoleToggled}, {"bindings_toggled", BindingsToggled},    {"settings_toggled", SettingsToggled}, {"tweakdbeditor_toggled", TweakDBEditorToggled},
            {"gamelog_toggled", GameLogToggled}, {"imguidebug_toggled", ImGuiDebugToggled}};
}

void ConsolePersistentState::Load(Options& aOptions, const nlohmann::json& aConfig)
{
    History = aConfig.value("history", History);

    if (History.size() > aOptions.Developer.MaxLinesConsoleHistory)
        History.erase(History.begin(), History.begin() + History.size() - aOptions.Developer.MaxLinesConsoleHistory);
}

nlohmann::json ConsolePersistentState::Save() const
{
    return {{"history", History}};
}

void PersistentState::Load()
{
    const auto path = GetAbsolutePath(m_paths.PersistentState(), "", false);
    if (path.empty())
        return;

    std::ifstream configFile(path);
    if (!configFile)
        return;

    auto state = nlohmann::json::parse(configFile, nullptr, false);

    if (state.is_discarded())
        return;

    const auto& overlayState = state["overlay"];
    if (!overlayState.empty())
        Overlay.Load(overlayState);

    if (!m_options.Developer.PersistentConsole)
        return;

    const auto& consoleState = state["console"];
    if (!consoleState.empty())
        Console.Load(m_options, consoleState);
}

void PersistentState::Save() const
{
    nlohmann::json state = {{"overlay", Overlay.Save()}};

    if (m_options.Developer.PersistentConsole)
        state["console"] = Console.Save();

    const auto path = GetAbsolutePath(m_paths.PersistentState(), "", true);
    std::ofstream o(path);
    o << state.dump(4) << std::endl;
}

PersistentState::PersistentState(Paths& aPaths, Options& aOptions)
    : m_paths(aPaths)
    , m_options(aOptions)
{
    Load();
    Save();

    GameMainThread::Get().AddShutdownTask(
        [this]
        {
            Save();
            return true;
        });
}
