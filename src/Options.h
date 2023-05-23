#pragma once

#include "Image.h"

struct Paths;

struct PatchesSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const PatchesSettings&) const = default;

    bool RemovePedestrians{false};
    bool AsyncCompute{false};
    bool Antialiasing{false};
    bool SkipStartMenu{false};
    bool AmdSmt{false};
    bool DisableIntroMovies{false};
    bool DisableVignette{false};
    bool DisableBoundaryTeleport{false};
    bool DisableWin7Vsync{false};
    bool MinimapFlicker{false};
};

struct LanguageSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const LanguageSettings&) const = default;

    std::string Locale{"en-US"};
};

struct FontSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const FontSettings&) const = default;

    std::string MainFont{"Default"};
    std::string MonoFont{"Default"};
    float BaseSize{18.0f};
    int32_t OversampleHorizontal{3};
    int32_t OversampleVertical{1};
};

struct DeveloperSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const DeveloperSettings&) const = default;

    bool RemoveDeadBindings{true};
    bool EnableImGuiAssertions{false};
    bool EnableDebug{false};
    bool DumpGameOptions{false};
    uint64_t MaxLinesConsoleHistory{1000};
    bool PersistentConsole{true};
};

struct Options
{
    Options(Paths& aPaths);
    ~Options() = default;

    void Load();
    void Save() const;
    void ResetToDefaults();

    Image GameImage;
    bool ExeValid{false};

    PatchesSettings Patches{};
    LanguageSettings Language{};
    FontSettings Font{};
    DeveloperSettings Developer{};

private:
    Paths& m_paths;
};
