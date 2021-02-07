#pragma once

#include "Widget.h"

struct VKBindings;
struct Overlay;
struct Options;

struct Settings : Widget
{
    Settings(Overlay& aOverlay, VKBindings& aBindings, Options& aOptions);
    ~Settings() override = default;

    void OnEnable() override;
    void OnDisable() override;
    void Update() override;
    
    void Load();
    void Save();
    void ResetToDefaults(); 

private:
    VKBindInfo m_overlayKeyBindInfo{ };
    bool m_patchEnableDebug{ false };
    bool m_patchRemovePedestrians{ false };
    bool m_patchAsyncCompute{ false };
    bool m_patchAntialiasing{ false };
    bool m_patchSkipStartMenu{ false };
    bool m_patchDisableIntroMovies{ false };
    bool m_patchDisableVignette{ false };
    bool m_patchDisableBoundaryTeleport{ false };
    bool m_patchDisableWin7Vsync{ false };
    bool m_dumpGameOptions{ false };
    bool m_telemetry{ true };
    VKBindings& m_bindings;
    Overlay& m_overlay;
    Options& m_options;
};
