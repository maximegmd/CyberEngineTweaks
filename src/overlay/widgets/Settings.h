#pragma once

#include "Widget.h"

struct Options;
struct LuaVM;
struct Settings : Widget
{
    Settings(Options& aOptions, LuaVM& aVm);
    ~Settings() override = default;

    WidgetResult OnEnable() override;
    WidgetResult OnDisable() override;

    void Load();
    void Save() const;
    void ResetToDefaults();

protected:
    void OnUpdate() override;
    WidgetResult OnPopup() override;

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
    bool m_enableImGuiAssertionsLogging{ false };

    Options& m_options;
    LuaVM& m_vm;

    TChangedCBResult m_popupResult{ TChangedCBResult::APPLY };
    bool m_madeChanges{ false };
    bool m_openChangesModal{ true };
};
