#pragma once

struct Paths;
struct Options;

struct OverlayPersistentState
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;

    bool ConsoleToggled = false;
    bool BindingsToggled = false;
    bool SettingsToggled = false;
    bool TweakDBEditorToggled = false;
    bool GameLogToggled = false;
    bool ImGuiDebugToggled = false;
};

struct ConsolePersistentState
{
    void Load(Options& aOptions, const nlohmann::json& aConfig);
    nlohmann::json Save() const;

    TiltedPhoques::Vector<std::string> History;
};

struct PersistentState
{
    PersistentState(Paths& aPaths, Options& aOptions);
    ~PersistentState();

    void Load();
    void Save() const;

    OverlayPersistentState Overlay{ };
    ConsolePersistentState Console{ };

private:

    Paths& m_paths;
    Options& m_options;
};
