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
    void SettingItemTemplate(const std::string& acIcon, const std::string& acLabel, const std::string& acTooltip, const bool& aValueChanged, std::function<void()>& aImGuiFunction);
    bool SettingItemCheckBox(const std::string& acIcon, const std::string& acLabel, const std::string& acTooltip, bool& aCurrent, const bool& acSaved);
    bool SettingItemSliderFloat(
        const std::string& acIcon, const std::string& acLabel, const std::string& acTooltip, float& aCurrent, const float& acSaved, float aValueMin, float aValueMax,
        const char* aFormat = "%.1f");
    bool SettingItemSliderInt(
        const std::string& acIcon, const std::string& acLabel, const std::string& acTooltip, int& aCurrent, const int& acSaved, int aValueMin, int aValueMax, const char* aFormat = "%d");
    bool SettingItemFontCombo(
        const std::string& acIcon, const std::string& acLabel, const std::string& acTooltip, std::string& aCurrent, const std::string& acSaved, const std::vector<Font>& acFonts);

    PatchesSettings m_patches;
    DeveloperSettings m_developer;
    FontSettings m_font;

    Options& m_options;
    LuaVM& m_vm;

    TChangedCBResult m_popupResult{TChangedCBResult::APPLY};
    bool m_madeChanges{false};
    bool m_madeFontChanges{false};
    bool m_openChangesModal{true};
};
