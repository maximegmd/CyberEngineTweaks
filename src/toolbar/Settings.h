#pragma once

#include "ToolbarWidget.h"

struct Settings : public ToolbarWidget
{
    Settings() = default;
    ~Settings() override = default;

    void OnEnable() override;
    void OnDisable() override;
    void Update() override;

    bool IsBindingKey() const;
    void RecordKeyDown(UINT aVKCode);
    void RecordKeyUp(UINT aVKCode);
    
    void Load();
    void Save();
    void ResetToDefaults(); 

protected:
    static const char* GetSpecialKeyName(UINT aVKCode);

private:
    UINT m_toolbarKey{ 0 };
    char m_toolbarChar{ 0 };
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
    
    bool m_wasBindingKey{ false };
    bool m_bindingKey{ false };
};
