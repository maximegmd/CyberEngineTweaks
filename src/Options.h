#pragma once

#include "Image.h"

struct Paths;

struct PatchesSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    bool RemovePedestrians{ false };
    bool AsyncCompute{ false };
    bool Antialiasing{ false };
    bool SkipStartMenu{ false };
    bool AmdSmt{ false };
    bool DisableIntroMovies{ false };
    bool DisableVignette{ false };
    bool DisableBoundaryTeleport{ false };
    bool DisableWin7Vsync{ false };
    bool MinimapFlicker{ false };
};

struct FontSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    std::string Path{ };
    std::string GlyphRanges{"Default"};
    float SizeBase{ 18.0f };
    int32_t OversampleHorizontal{ 3 };
    int32_t OversampleVertical{ 1 };
};

struct DeveloperSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    bool RemoveDeadBindings{ true };
    bool EnableImGuiAssertions{ false };
    bool EnableDebug{ false };
    bool DumpGameOptions{ false };
};

struct OverlayPersistentState
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    bool ConsoleToggled = false;
    bool BindingsToggled = false;
    bool SettingsToggled = false;
    bool TweakDBEditorToggled = false;
    bool GameLogToggled = false;
    bool ImGuiDebugToggled = false;
};

struct PersistentState
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    OverlayPersistentState Overlay{ };
};

struct Options
{
    Options(Paths& aPaths);
    ~Options();

    void Load(const bool acPersistentStateReload = false);
    void Save() const;
    void ResetToDefaults();

    Image GameImage;
    bool ExeValid{ false };

    PatchesSettings Patches{ };
    FontSettings Font{ };
    DeveloperSettings Developer{ };
    PersistentState PersistentState{ };

private:

    Paths& m_paths;
};