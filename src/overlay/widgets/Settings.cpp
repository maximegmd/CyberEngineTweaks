#include <stdafx.h>

#include "Settings.h"

#include "HelperWidgets.h"

#include <overlay/Overlay.h>

void Settings::OnEnable()
{
    Load();
    
    VKBindings::Get().StopRecordingBind();
}

void Settings::OnDisable()
{
    VKBindings::Get().StopRecordingBind();
}

void Settings::Update()
{
    if (ImGui::Button("Load"))
        Load();
    ImGui::SameLine();
    if (ImGui::Button("Save"))
        Save();
    ImGui::SameLine();
    if (ImGui::Button("Defaults"))
        ResetToDefaults();

    ImGui::Spacing();

    auto& options = Options::Get();

    if (!options.IsFirstLaunch)
        ImGui::BeginChild("##SETTINGS_ACTUAL", ImVec2(0,0), true);

    HelperWidgets::BindWidget(m_overlayKeyBindInfo);
    if (options.IsFirstLaunch && (m_overlayKeyBindInfo.SavedCodeBind != m_overlayKeyBindInfo.CodeBind))
    {
        Save();
        Load();
    }

    HelperWidgets::BoolWidget("Enable Debug Menu:", m_patchEnableDebug, options.PatchEnableDebug);
    HelperWidgets::BoolWidget("Remove Pedestrians:", m_patchRemovePedestrians, options.PatchRemovePedestrians);
    HelperWidgets::BoolWidget("Disable Async Compute:", m_patchAsyncCompute, options.PatchAsyncCompute);
    HelperWidgets::BoolWidget("Disable Antialiasing:", m_patchAntialiasing, options.PatchAntialiasing);
    HelperWidgets::BoolWidget("Skip Start Menu:", m_patchSkipStartMenu, options.PatchSkipStartMenu);
    HelperWidgets::BoolWidget("Suppress Intro Movies:", m_patchDisableIntroMovies, options.PatchDisableIntroMovies);
    HelperWidgets::BoolWidget("Disable Vignette:", m_patchDisableVignette, options.PatchDisableVignette);
    HelperWidgets::BoolWidget("Disable Boundary Teleport:", m_patchDisableBoundaryTeleport, options.PatchDisableBoundaryTeleport);
    HelperWidgets::BoolWidget("Disable V-Sync (Windows 7 only):", m_patchDisableWin7Vsync, options.PatchDisableWin7Vsync);
    HelperWidgets::BoolWidget("Dump Game Options:", m_dumpGameOptions, options.DumpGameOptions);

    if (!options.IsFirstLaunch)
        ImGui::EndChild();
}

void Settings::Load()
{
    auto& options = Options::Get();

    options.Load();

    m_overlayKeyBindInfo.Fill(options.OverlayKeyBind, Overlay::VKBOverlay);
    m_patchEnableDebug = options.PatchEnableDebug;
    m_patchRemovePedestrians = options.PatchRemovePedestrians;
    m_patchAsyncCompute = options.PatchAsyncCompute;
    m_patchAntialiasing = options.PatchAntialiasing;
    m_patchSkipStartMenu = options.PatchSkipStartMenu;
    m_patchDisableIntroMovies = options.PatchDisableIntroMovies;
    m_patchDisableVignette = options.PatchDisableVignette;
    m_patchDisableBoundaryTeleport = options.PatchDisableBoundaryTeleport;
    m_patchDisableWin7Vsync = options.PatchDisableWin7Vsync;
    m_dumpGameOptions = options.DumpGameOptions;
}

void Settings::Save()
{
    auto& options = Options::Get();

    if (m_overlayKeyBindInfo.SavedCodeBind != m_overlayKeyBindInfo.CodeBind)
    {
        options.OverlayKeyBind = m_overlayKeyBindInfo.Apply();
        VKBindings::Get().Save(); // also save bindings in this case!
    }

    options.PatchEnableDebug = m_patchEnableDebug;
    options.PatchRemovePedestrians = m_patchRemovePedestrians;
    options.PatchAsyncCompute = m_patchAsyncCompute;
    options.PatchAntialiasing = m_patchAntialiasing;
    options.PatchSkipStartMenu = m_patchSkipStartMenu;
    options.PatchDisableIntroMovies = m_patchDisableIntroMovies;
    options.PatchDisableVignette = m_patchDisableVignette;
    options.PatchDisableBoundaryTeleport = m_patchDisableBoundaryTeleport;
    options.PatchDisableWin7Vsync = m_patchDisableWin7Vsync;
    options.DumpGameOptions = m_dumpGameOptions;

    options.Save();
}

void Settings::ResetToDefaults()
{
    Options::Get().ResetToDefaults();
    Load();
}
