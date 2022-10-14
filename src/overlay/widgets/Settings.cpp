#include <stdafx.h>

#include "Settings.h"

#include <CET.h>

#include <Utils.h>

Settings::Settings(Options& aOptions, LuaVM& aVm)
    : m_options(aOptions)
    , m_vm(aVm)
{
}

WidgetResult Settings::OnEnable()
{
    if (!m_enabled)
    {
        Load();
        m_enabled = true;
    }
    return m_enabled ? WidgetResult::ENABLED : WidgetResult::DISABLED;
}

WidgetResult Settings::OnDisable()
{
    WidgetResult result = WidgetResult::ENABLED;

    if (m_enabled)
    {
        m_vm.BlockDraw(m_madeChanges);
        const auto ret = UnsavedChangesPopup(
            m_openChangesModal,
            m_madeChanges,
            [this]{ Save(); },
            [this]{ Load(); });
        m_madeChanges = ret == THWUCPResult::CHANGED;
        m_vm.BlockDraw(m_madeChanges);

        m_enabled = m_madeChanges;
        if (ret == THWUCPResult::CANCEL)
        {
            CET::Get().GetOverlay().SetActiveWidget(WidgetID::SETTINGS);
            m_enabled = true;
            result = WidgetResult::CANCEL;
        }
    }
    if (!m_enabled)
    {
        // reset changes substates
        m_madeChanges = false;
    }

    if (result != WidgetResult::CANCEL)
        result = m_enabled ? WidgetResult::ENABLED : WidgetResult::DISABLED;

    return result;
}

void Settings::Update()
{
    const auto frameSize = ImVec2(ImGui::GetContentRegionAvail().x, -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y + ImGui::GetStyle().FramePadding.y + 2.0f));
    if (ImGui::BeginChild(ImGui::GetID("Settings"), frameSize))
    {
        m_madeChanges = false;
        if (ImGui::CollapsingHeader("Patches", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePush();
            if (ImGui::BeginTable("##SETTINGS_PATCHES", 2, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders, ImVec2(-ImGui::GetStyle().IndentSpacing, 0)))
            {
                UpdateAndDrawSetting("AMD SMT Patch", "For AMD CPUs that did not get a performance boost after CDPR's patch (requires restart to take effect).", m_patchAmdSmt, m_options.PatchAmdSmt);
                UpdateAndDrawSetting("Remove Pedestrians", "Removes most of the pedestrians and traffic (requires restart to take effect).", m_patchRemovePedestrians, m_options.PatchRemovePedestrians);
                UpdateAndDrawSetting("Disable Async Compute", "Disables async compute, this can give a boost on older GPUs like Nvidia 10xx series for example (requires restart to take effect).", m_patchAsyncCompute, m_options.PatchAsyncCompute);
                UpdateAndDrawSetting("Disable Anti-aliasing", "Completely disables anti-aliasing (requires restart to take effect).", m_patchAntialiasing, m_options.PatchAntialiasing);
                UpdateAndDrawSetting("Skip Start Menu", "Skips the 'Breaching...' menu asking you to press space bar to continue (requires restart to take effect).", m_patchSkipStartMenu, m_options.PatchSkipStartMenu);
                UpdateAndDrawSetting("Suppress Intro Movies", "Disables logos played at the beginning (requires restart to take effect).", m_patchDisableIntroMovies, m_options.PatchDisableIntroMovies);
                UpdateAndDrawSetting("Disable Vignette", "Disables vignetting along screen borders (requires restart to take effect).", m_patchDisableVignette, m_options.PatchDisableVignette);
                UpdateAndDrawSetting("Disable Boundary Teleport", "Allows players to access out-of-bounds locations (requires restart to take effect).", m_patchDisableBoundaryTeleport, m_options.PatchDisableBoundaryTeleport);
                UpdateAndDrawSetting("Disable V-Sync (Windows 7 only)", " (requires restart to take effect).", m_patchDisableWin7Vsync, m_options.PatchDisableWin7Vsync);
                UpdateAndDrawSetting("Fix Minimap Flicker", "Disables VSync on Windows 7 to bypass the 60 FPS limit (requires restart to take effect).", m_patchMinimapFlicker, m_options.PatchMinimapFlicker);

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        if (ImGui::CollapsingHeader("Dev", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePush();
            if (ImGui::BeginTable("##SETTINGS_DEV", 2, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders, ImVec2(-ImGui::GetStyle().IndentSpacing, 0)))
            {
                UpdateAndDrawSetting("Draw ImGui Diagnostic Window", "Toggles drawing of internal ImGui diagnostics window to show what is going on behind the scenes (useful when debugging ImGui issues).", m_options.DrawImGuiDiagnosticWindow, m_options.DrawImGuiDiagnosticWindow);
                UpdateAndDrawSetting("Remove Dead Bindings", "Removes all bindings which are no longer valid (disabling this could be useful when debugging mod issues).", m_removeDeadBindings, m_options.RemoveDeadBindings);
                UpdateAndDrawSetting("Enable ImGui Assertions", "Enables all ImGui assertions (useful when debugging ImGui issues, should also be used to check mods before shipping!).", m_enableImGuiAssertions, m_options.EnableImGuiAssertions);
                UpdateAndDrawSetting("Enable Debug Build", "Sets internal flags to disguise as debug build (requires restart to take effect).", m_patchEnableDebug, m_options.PatchEnableDebug);
                UpdateAndDrawSetting("Dump Game Options", "Dumps all game options into main log file (requires restart to take effect).", m_dumpGameOptions, m_options.DumpGameOptions);

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

    m_patchRemovePedestrians = m_options.PatchRemovePedestrians;
    m_patchAsyncCompute = m_options.PatchAsyncCompute;
    m_patchAntialiasing = m_options.PatchAntialiasing;
    m_patchSkipStartMenu = m_options.PatchSkipStartMenu;
    m_patchAmdSmt = m_options.PatchAmdSmt;
    m_patchDisableIntroMovies = m_options.PatchDisableIntroMovies;
    m_patchDisableVignette = m_options.PatchDisableVignette;
    m_patchDisableBoundaryTeleport = m_options.PatchDisableBoundaryTeleport;
    m_patchDisableWin7Vsync = m_options.PatchDisableWin7Vsync;
    m_patchMinimapFlicker = m_options.PatchMinimapFlicker;

    m_removeDeadBindings = m_options.RemoveDeadBindings;
    m_enableImGuiAssertions = m_options.EnableImGuiAssertions;
    m_patchEnableDebug = m_options.PatchEnableDebug;
    m_dumpGameOptions = m_options.DumpGameOptions;
}

void Settings::Save() const
{
    m_options.PatchRemovePedestrians = m_patchRemovePedestrians;
    m_options.PatchAsyncCompute = m_patchAsyncCompute;
    m_options.PatchAntialiasing = m_patchAntialiasing;
    m_options.PatchSkipStartMenu = m_patchSkipStartMenu;
    m_options.PatchAmdSmt = m_patchAmdSmt;
    m_options.PatchDisableIntroMovies = m_patchDisableIntroMovies;
    m_options.PatchDisableVignette = m_patchDisableVignette;
    m_options.PatchDisableBoundaryTeleport = m_patchDisableBoundaryTeleport;
    m_options.PatchDisableWin7Vsync = m_patchDisableWin7Vsync;
    m_options.PatchMinimapFlicker = m_patchMinimapFlicker;

    m_options.RemoveDeadBindings = m_removeDeadBindings;
    m_options.EnableImGuiAssertions = m_enableImGuiAssertions;
    m_options.PatchEnableDebug = m_patchEnableDebug;
    m_options.DumpGameOptions = m_dumpGameOptions;

    m_options.Save();
}

void Settings::ResetToDefaults()
{
    m_options.ResetToDefaults();
    Load();
}

void Settings::UpdateAndDrawSetting(const std::string& acLabel, const std::string& acTooltip, bool& aCurrent, const bool& acSaved)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImVec4 curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (aCurrent != acSaved)
        curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    ImGui::AlignTextToFramePadding();

    ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);

    ImGui::PushID(&acLabel);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + GetCenteredOffsetForText(acLabel.c_str()));
    ImGui::TextUnformatted(acLabel.c_str());
    ImGui::PopID();

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !acTooltip.empty())
        ImGui::SetTooltip("%s", acTooltip.c_str());

    ImGui::TableNextColumn();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::GetFrameHeight()) / 2);
    ImGui::Checkbox(("##" + acLabel).c_str(), &aCurrent);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", acTooltip.c_str());

    ImGui::PopStyleColor();

    m_madeChanges |= aCurrent != acSaved;
}
