#include <stdafx.h>

#include "Settings.h"

#include <CET.h>

#include <Utils.h>

Settings::Settings(Options& aOptions, LuaVM& aVm)
    : Widget(ICON_MD_COG " Settings")
    , m_options(aOptions)
    , m_vm(aVm)
{
    Load();
}

WidgetResult Settings::OnPopup()
{
    const auto ret = UnsavedChangesPopup(
        "Settings", m_openChangesModal, m_madeChanges, [this] { Save(); }, [this] { Load(); });
    m_madeChanges = ret == TChangedCBResult::CHANGED;
    m_popupResult = ret;

    return m_madeChanges ? WidgetResult::ENABLED : WidgetResult::DISABLED;
}

WidgetResult Settings::OnDisable()
{
    if (m_enabled)
    {
        if (m_popupResult == TChangedCBResult::CANCEL)
        {
            m_popupResult = TChangedCBResult::APPLY;
            return WidgetResult::CANCEL;
        }

        if (m_madeChanges)
        {
            m_drawPopup = true;
            return WidgetResult::ENABLED;
        }

        m_enabled = false;
    }

    return m_enabled ? WidgetResult::ENABLED : WidgetResult::DISABLED;
}

void Settings::OnUpdate()
{
    const auto frameSize = ImVec2(ImGui::GetContentRegionAvail().x, -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y + ImGui::GetStyle().FramePadding.y + 2.0f));
    if (ImGui::BeginChild(ImGui::GetID("Settings"), frameSize))
    {
        m_madeChanges = false;
        m_madeFontChanges = false;
        if (ImGui::CollapsingHeader("Patches", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePush();
            if (ImGui::BeginTable("SETTINGS", 2, ImGuiTableFlags_NoSavedSettings, ImVec2(-ImGui::GetStyle().IndentSpacing, 0)))
            {
                const auto& patchesSettings = m_options.Patches;
                SettingItemCheckBox(
                    "🚑", "AMD SMT Patch", "For AMD CPUs that did not get a performance boost after CDPR's patch (requires restart to take effect).", m_patches.AmdSmt,
                    patchesSettings.AmdSmt);
                SettingItemCheckBox(
                    "👻", "Remove Pedestrians", "Removes most of the pedestrians and traffic (requires restart to take effect).", m_patches.RemovePedestrians,
                    patchesSettings.RemovePedestrians);
                SettingItemCheckBox(
                    "🐌", "Disable Async Compute",
                    "Disables async compute, this can give a boost on older GPUs like Nvidia 10xx series for example (requires restart to take effect).", m_patches.AsyncCompute,
                    patchesSettings.AsyncCompute);
                SettingItemCheckBox(
                    "🤮", "Disable Anti-aliasing", "Completely disables anti-aliasing (requires restart to take effect).", m_patches.Antialiasing, patchesSettings.Antialiasing);
                SettingItemCheckBox(
                    "🏄", "Skip Start Menu", "Skips the 'Breaching...' menu asking you to press space bar to continue (requires restart to take effect).", m_patches.SkipStartMenu,
                    patchesSettings.SkipStartMenu);
                SettingItemCheckBox(
                    "🎞", "Suppress Intro Movies", "Disables logos played at the beginning (requires restart to take effect).", m_patches.DisableIntroMovies,
                    patchesSettings.DisableIntroMovies);
                SettingItemCheckBox(
                    "🔦", "Disable Vignette", "Disables vignetting along screen borders (requires restart to take effect).", m_patches.DisableVignette,
                    patchesSettings.DisableVignette);
                SettingItemCheckBox(
                    "🗺", "Disable Boundary Teleport", "Allows players to access out-of-bounds locations (requires restart to take effect).", m_patches.DisableBoundaryTeleport,
                    patchesSettings.DisableBoundaryTeleport);
                SettingItemCheckBox("💨", "Disable V-Sync (Windows 7 only)", " (requires restart to take effect).", m_patches.DisableWin7Vsync, patchesSettings.DisableWin7Vsync);
                SettingItemCheckBox(
                    "✨", "Fix Minimap Flicker", "Disables VSync on Windows 7 to bypass the 60 FPS limit (requires restart to take effect).", m_patches.MinimapFlicker,
                    patchesSettings.MinimapFlicker);

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        if (ImGui::CollapsingHeader("CET Font Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePush();
            if (ImGui::BeginTable("SETTINGS", 2, ImGuiTableFlags_NoSavedSettings, ImVec2(-ImGui::GetStyle().IndentSpacing, 0)))
            {
                const auto& fontSettings = m_options.Font;
                m_madeFontChanges |=
                    SettingItemCombo("🦄", "Main Font", "Main display font for CET.", m_font.FontMain, fontSettings.FontMain, CET::Get().GetFonts().GetSystemFonts());
                m_madeFontChanges |= SettingItemCombo(
                    "🪲", "Monospace Font", "Monospacee font, which is used for displaying texts in Console and Game Log, for CET.", m_font.FontMonospace,
                    fontSettings.FontMonospace, CET::Get().GetFonts().GetSystemFonts());
                m_madeFontChanges |= SettingItemSliderFloat(
                    "📏", "Font Size", "Changees the size of the font, default value is 18px.", m_font.BaseSize, fontSettings.BaseSize, 10.0f, 72.0f, "%.0fpx");

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                static bool openFontAdvSettings = false;
                ImGui::Selectable("Advance Settings", false, ImGuiSelectableFlags_SpanAllColumns);
                if (ImGui::IsItemClicked())
                    openFontAdvSettings = !openFontAdvSettings;
                if (openFontAdvSettings)
                {
                    ImGui::Indent(ImGui::GetFrameHeight());
                    m_madeFontChanges |= SettingItemSliderInt(
                        "↔", "Oversample Horizontal", "Oversamples font horizontally, default value is 3x. (May increase font clearity, at the cost of increasing memory usage.)",
                        m_font.OversampleHorizontal, fontSettings.OversampleHorizontal, 1, 10, "%dx");
                    m_madeFontChanges |= SettingItemSliderInt(
                        "↕", "Oversample Vertical", "Oversamples font vertically, default value is 1x. (May increase font clearity, at the cost of increasing memory usage.)",
                        m_font.OversampleVertical, fontSettings.OversampleVertical, 1, 10, "%dx");
                    ImGui::Unindent(ImGui::GetFrameHeight());
                }

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        if (ImGui::CollapsingHeader("CET Development Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePush();
            if (ImGui::BeginTable("SETTINGS", 2, ImGuiTableFlags_NoSavedSettings, ImVec2(-ImGui::GetStyle().IndentSpacing, 0)))
            {
                const auto& developerSettings = m_options.Developer;
                SettingItemCheckBox(
                    "🗑", "Remove Dead Bindings", "Removes all bindings which are no longer valid (disabling this could be useful when debugging mod issues).",
                    m_developer.RemoveDeadBindings, developerSettings.RemoveDeadBindings);
                SettingItemCheckBox(
                    "💣", "Enable ImGui Assertions",
                    "Enables all ImGui assertions, assertions will get logged into log file of whoever triggered the assertion (useful when debugging ImGui issues, should also be "
                    "used to check mods before shipping!).",
                    m_developer.EnableImGuiAssertions, developerSettings.EnableImGuiAssertions);
                SettingItemCheckBox(
                    "🔨", "Enable Debug Build", "Sets internal flags to disguise as debug build (requires restart to take effect).", m_developer.EnableDebug,
                    developerSettings.EnableDebug);
                SettingItemCheckBox(
                    "🖨", "Dump Game Options", "Dumps all game options into main log file (requires restart to take effect).", m_developer.DumpGameOptions,
                    developerSettings.DumpGameOptions);

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();

    ImGui::Separator();

    const auto itemWidth = GetAlignedItemWidth(3);
    if (ImGui::Button("Load", ImVec2(itemWidth, 0)))
        Load();
    ImGui::SameLine();
    if (ImGui::Button("Save", ImVec2(itemWidth, 0)))
        Save();
    ImGui::SameLine();
    if (ImGui::Button("Defaults", ImVec2(itemWidth, 0)))
        ResetToDefaults();
}

void Settings::Load()
{
    m_options.Load();

    m_patches = m_options.Patches;
    m_developer = m_options.Developer;
    m_font = m_options.Font;
}

void Settings::Save() const
{
    m_options.Patches = m_patches;
    m_options.Developer = m_developer;
    m_options.Font = m_font;
    if (m_madeFontChanges)
        CET::Get().GetFonts().RebuildFontNextFrame();

    m_options.Save();
}

void Settings::ResetToDefaults()
{
    m_options.ResetToDefaults();

    m_patches = m_options.Patches;
    m_developer = m_options.Developer;
    m_font = m_options.Font;
    CET::Get().GetFonts().RebuildFontNextFrame();
}

void Settings::SettingItemTemplate(const std::string& acIcon, const std::string& acLabel, const std::string& acTooltip, const bool& aValueChanged, std::function<void()>& aImGuiFunction)
{
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);

    ImVec4 curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (aValueChanged)
        curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    ImGui::AlignTextToFramePadding();

    ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);

    ImGui::PushID(&acLabel);
    if (CET::Get().GetFonts().UseEmojiFont() && !acIcon.empty())
    {
        CET::Get().GetFonts().GetGlyphRangesBuilder().AddText(acIcon);
        ImGui::TextUnformatted(acIcon.c_str());
        ImGui::SameLine();
    }
    ImGui::TextUnformatted(acLabel.c_str());
    ImGui::PopID();

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !acTooltip.empty())
        ImGui::SetTooltip("%s", acTooltip.c_str());

    ImGui::TableSetColumnIndex(1);

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x / 4);
    ImGui::SetNextItemWidth(-FLT_MIN);

    aImGuiFunction();

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", acTooltip.c_str());

    ImGui::PopStyleColor();
}

bool Settings::SettingItemCheckBox(const std::string& acIcon, const std::string& acLabel, const std::string& acTooltip, bool& aCurrent, const bool& acSaved)
{
    std::function<void()> imguiWidget = [&]()
    {
        // Right align checkbox
        if (ImGui::BeginTable("CheckboxItem", 2, ImGuiTableFlags_NoSavedSettings))
        {
            ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(1);

            ImGui::Checkbox(("##" + acLabel).c_str(), &aCurrent);

            ImGui::EndTable();
        }
    };

    const bool valueChanged = aCurrent != acSaved;

    SettingItemTemplate(acIcon, acLabel, acTooltip, valueChanged, imguiWidget);

    m_madeChanges |= aCurrent != acSaved;

    return aCurrent != acSaved;
}

bool Settings::SettingItemSliderFloat(
    const std::string& acIcon, const std::string& acLabel, const std::string& acTooltip, float& aCurrent, const float& acSaved, float aValueMin, float aValueMax, const char* aFormat)
{
    std::function<void()> imguiWidget = [&]()
    {
        ImGui::SliderFloat(("##" + acLabel).c_str(), &aCurrent, aValueMin, aValueMax, aFormat);
    };

    const bool valueChanged = aCurrent != acSaved;

    SettingItemTemplate(acIcon, acLabel, acTooltip, valueChanged, imguiWidget);

    m_madeChanges |= aCurrent != acSaved;

    return aCurrent != acSaved;
}

bool Settings::SettingItemSliderInt(
    const std::string& acIcon, const std::string& acLabel, const std::string& acTooltip, int& aCurrent, const int& acSaved, int aValueMin, int aValueMax, const char* aFormat)
{
    std::function<void()> imguiWidget = [&]()
    {
        ImGui::SliderInt(("##" + acLabel).c_str(), &aCurrent, aValueMin, aValueMax, aFormat);
    };

    const bool valueChanged = aCurrent != acSaved;

    SettingItemTemplate(acIcon, acLabel, acTooltip, valueChanged, imguiWidget);

    m_madeChanges |= aCurrent != acSaved;

    return aCurrent != acSaved;
}

bool Settings::SettingItemCombo(
    const std::string& acIcon, const std::string& acLabel, const std::string& acTooltip, std::string& aCurrent, const std::string& acSaved, const std::vector<std::string>& acItems)
{
    std::function<void()> imguiWidget = [&]()
    {
        if (ImGui::BeginCombo(("##" + acLabel).c_str(), aCurrent.c_str()))
        {
            for (const auto item : acItems)
            {
                const bool isSelected = aCurrent == item;
                if (ImGui::Selectable(item.c_str(), isSelected))
                    aCurrent = item;
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    };

    const bool valueChanged = aCurrent != acSaved;

    SettingItemTemplate(acIcon, acLabel, acTooltip, valueChanged, imguiWidget);

    m_madeChanges |= aCurrent != acSaved;

    return aCurrent != acSaved;
}