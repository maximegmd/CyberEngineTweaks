#pragma once

#include "Widget.h"

struct VKBindings;
struct Overlay;
struct Options;

struct Settings : Widget
{
    Settings(Options& aOptions);
    ~Settings() override = default;

    bool OnEnable() override;
    bool OnDisable() override;
    void Update() override;
    
    void Load();
    void Save() const;
    void ResetToDefaults(); 

private:
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
    bool m_dumpGameOptions{ false };
    bool m_drawImGuiDiagnosticWindow{ false };
    bool m_removeDeadBindings{ true };
    Options& m_options;

    bool m_enabled{ false };
    bool m_madeChanges{ false };
    bool m_showChangesModal{ false };
};
