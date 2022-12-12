#include <stdafx.h>

#include "Settings.h"

#include "EngineTweaks.h"

#include <Utils.h>

Settings::Settings(Options& aOptions, LuaVM& aVm)
    : Widget("Settings")
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
        if (ImGui::CollapsingHeader("Patches", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePush();
            if (ImGui::BeginTable("##SETTINGS_PATCHES", 2, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchSame, ImVec2(-ImGui::GetStyle().IndentSpacing, 0)))
            {
                const auto& patchesSettings = m_options.Patches;
                UpdateAndDrawSetting(
                    "AMD SMT Patch",
                    "For AMD CPUs that did not get a performance boost after CDPR's patch (requires restart to take "
                    "effect).",
                    m_patches.AmdSmt, patchesSettings.AmdSmt);
                UpdateAndDrawSetting(
                    "Remove Pedestrians", "Removes most of the pedestrians and traffic (requires restart to take effect).", m_patches.RemovePedestrians,
                    patchesSettings.RemovePedestrians);
                UpdateAndDrawSetting(
                    "Disable Async Compute",
                    "Disables async compute, this can give a boost on older GPUs like Nvidia 10xx series for example "
                    "(requires restart to take effect).",
                    m_patches.AsyncCompute, patchesSettings.AsyncCompute);
                UpdateAndDrawSetting(
                    "Disable Anti-aliasing", "Completely disables anti-aliasing (requires restart to take effect).", m_patches.Antialiasing, patchesSettings.Antialiasing);
                UpdateAndDrawSetting(
                    "Skip Start Menu",
                    "Skips the 'Breaching...' menu asking you to press space bar to continue (requires restart to take "
                    "effect).",
                    m_patches.SkipStartMenu, patchesSettings.SkipStartMenu);
                UpdateAndDrawSetting(
                    "Suppress Intro Movies", "Disables logos played at the beginning (requires restart to take effect).", m_patches.DisableIntroMovies,
                    patchesSettings.DisableIntroMovies);
                UpdateAndDrawSetting(
                    "Disable Vignette", "Disables vignetting along screen borders (requires restart to take effect).", m_patches.DisableVignette, patchesSettings.DisableVignette);
                UpdateAndDrawSetting(
                    "Disable Boundary Teleport", "Allows players to access out-of-bounds locations (requires restart to take effect).", m_patches.DisableBoundaryTeleport,
                    patchesSettings.DisableBoundaryTeleport);
                UpdateAndDrawSetting("Disable V-Sync (Windows 7 only)", " (requires restart to take effect).", m_patches.DisableWin7Vsync, patchesSettings.DisableWin7Vsync);
                UpdateAndDrawSetting(
                    "Fix Minimap Flicker", "Disables VSync on Windows 7 to bypass the 60 FPS limit (requires restart to take effect).", m_patches.MinimapFlicker,
                    patchesSettings.MinimapFlicker);

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        if (ImGui::CollapsingHeader("CET Development Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePush();
            if (ImGui::BeginTable("##SETTINGS_DEV", 2, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchSame, ImVec2(-ImGui::GetStyle().IndentSpacing, 0)))
            {
                const auto& developerSettings = m_options.Developer;
                UpdateAndDrawSetting(
                    "Remove Dead Bindings",
                    "Removes all bindings which are no longer valid (disabling this could be useful when debugging mod "
                    "issues).",
                    m_developer.RemoveDeadBindings, developerSettings.RemoveDeadBindings);
                UpdateAndDrawSetting(
                    "Enable ImGui Assertions",
                    "Enables all ImGui assertions, assertions will get logged into log file of whoever triggered the "
                    "assertion (useful when debugging ImGui issues, should also be used to check mods before "
                    "shipping!).",
                    m_developer.EnableImGuiAssertions, developerSettings.EnableImGuiAssertions);
                UpdateAndDrawSetting(
                    "Enable Debug Build", "Sets internal flags to disguise as debug build (requires restart to take effect).", m_developer.EnableDebug,
                    developerSettings.EnableDebug);
                UpdateAndDrawSetting(
                    "Dump Game Options", "Dumps all game options into main log file (requires restart to take effect).", m_developer.DumpGameOptions,
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
}

void Settings::Save() const
{
    m_options.Patches = m_patches;
    m_options.Developer = m_developer;

    m_options.Save();
}

void Settings::ResetToDefaults()
{
    m_options.ResetToDefaults();

    m_patches = m_options.Patches;
    m_developer = m_options.Developer;
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
