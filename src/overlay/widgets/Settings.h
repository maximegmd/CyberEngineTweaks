#pragma once

#include "Widget.h"

struct VKBindings;
struct Overlay;
struct Options;
struct LuaVM;

struct Settings : Widget
{
    Settings(Options& aOptions, LuaVM& aVm);
    ~Settings() override = default;

    WidgetResult OnEnable() override;
    WidgetResult OnDisable() override;
    void Update() override;

    void Load();
    void Save() const;
    void ResetToDefaults();

private:
    void UpdateAndDrawSetting(const std::string& acLabel, const std::string& acTooltip, bool& aCurrent, const bool& acSaved);

    bool m_patchEnableDebug{ false };
    bool m_patchRemovePedestrians{ false };
    bool m_patchAsyncCompute{ false };
    bool m_patchAntialiasing{ false };
    bool m_patchSkipStartMenu{ false };
    bool m_patchAmdSmt{ false };
    bool m_patchDisableIntroMovies{ false };
    bool m_patchDisableVignette{ false };
    bool m_patchDisableBoundaryTeleport{ false };
    bool m_patchDisableWin7Vsync{ false };
    bool m_patchMinimapFlicker{ false };
    bool m_dumpGameOptions{ false };
    bool m_removeDeadBindings{ true };
    bool m_enableImGuiAssertions{ true };
    bool m_packageTweakDBStr{ false };

    Options& m_options;
    LuaVM& m_vm;

    bool m_enabled{ false };
    bool m_madeChanges{ false };
    bool m_openChangesModal{ true };
};
