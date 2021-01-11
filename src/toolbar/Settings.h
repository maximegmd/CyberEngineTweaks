#pragma once

#include "ToolbarWidget.h"

struct Settings : public ToolbarWidget
{
    Settings() = default;
    ~Settings() override = default;

    void OnEnable() override;
    void OnDisable() override;
    void Update() override;
    
    void Load();
    void Save();
    void ResetToDefaults(); 

private:
    VKBindInfo m_toolbarKeyBindInfo{ };
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
    
    bool m_bindingKey{ false };
};
