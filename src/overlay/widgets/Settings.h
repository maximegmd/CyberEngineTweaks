#pragma once

#include "Widget.h"

#include <Options.h>

struct LuaVM;
struct Settings : Widget
{
    Settings(Options& aOptions, LuaVM& aVm);
    ~Settings() override = default;

    WidgetResult OnDisable() override;

    void Load();
    void Save() const;
    void ResetToDefaults();

protected:
    void OnUpdate() override;
    WidgetResult OnPopup() override;

private:
    void UpdateAndDrawSetting(const std::string& acLabel, const std::string& acTooltip, bool& aCurrent, const bool& acSaved);

    PatchesSettings m_patches;
    DeveloperSettings m_developer;

    Options& m_options;
    LuaVM& m_vm;

    TChangedCBResult m_popupResult{TChangedCBResult::APPLY};
    bool m_madeChanges{false};
    bool m_openChangesModal{true};
};
