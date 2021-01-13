#include <stdafx.h>

#include "Settings.h"

#include "HelperWidgets.h"

#include <overlay/Overlay.h>

void Settings::OnEnable()
{
    Load();
    
    VKBindings::StopRecordingBind();
}

void Settings::OnDisable()
{
    VKBindings::StopRecordingBind();
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

    if (!Options::IsFirstLaunch)
        ImGui::BeginChild("##SETTINGS_ACTUAL", ImVec2(0,0), true);

    HelperWidgets::BindWidget(m_overlayKeyBindInfo);
    if (Options::IsFirstLaunch && (m_overlayKeyBindInfo.SavedCodeBind != m_overlayKeyBindInfo.CodeBind))
    {
        Save();
        Load();
    }

    HelperWidgets::BoolWidget("Enable Debug Menu:", m_patchEnableDebug, Options::PatchEnableDebug);
    HelperWidgets::BoolWidget("Remove Pedestrians:", m_patchRemovePedestrians, Options::PatchRemovePedestrians);
    HelperWidgets::BoolWidget("Disable Async Compute:", m_patchAsyncCompute, Options::PatchAsyncCompute);
    HelperWidgets::BoolWidget("Disable Antialiasing:", m_patchAntialiasing, Options::PatchAntialiasing);
    HelperWidgets::BoolWidget("Skip Start Menu:", m_patchSkipStartMenu, Options::PatchSkipStartMenu);
    HelperWidgets::BoolWidget("Suppress Intro Movies:", m_patchDisableIntroMovies, Options::PatchDisableIntroMovies);
    HelperWidgets::BoolWidget("Disable Vignette:", m_patchDisableVignette, Options::PatchDisableVignette);
    HelperWidgets::BoolWidget("Disable Boundary Teleport:", m_patchDisableBoundaryTeleport, Options::PatchDisableBoundaryTeleport);
    HelperWidgets::BoolWidget("Disable V-Sync (Windows 7 only):", m_patchDisableWin7Vsync, Options::PatchDisableWin7Vsync);
    HelperWidgets::BoolWidget("Dump Game Options:", m_dumpGameOptions, Options::DumpGameOptions);

    if (!Options::IsFirstLaunch)
        ImGui::EndChild();
}

void Settings::Load()
{
    Options::Load();

    m_overlayKeyBindInfo.Fill(Options::OverlayKeyBind, Overlay::VKBOverlay);
    m_patchEnableDebug = Options::PatchEnableDebug;
    m_patchRemovePedestrians = Options::PatchRemovePedestrians;
    m_patchAsyncCompute = Options::PatchAsyncCompute;
    m_patchAntialiasing = Options::PatchAntialiasing;
    m_patchSkipStartMenu = Options::PatchSkipStartMenu;
    m_patchDisableIntroMovies = Options::PatchDisableIntroMovies;
    m_patchDisableVignette = Options::PatchDisableVignette;
    m_patchDisableBoundaryTeleport = Options::PatchDisableBoundaryTeleport;
    m_patchDisableWin7Vsync = Options::PatchDisableWin7Vsync;
    m_dumpGameOptions = Options::DumpGameOptions;
}

void Settings::Save()
{
    if (m_overlayKeyBindInfo.SavedCodeBind != m_overlayKeyBindInfo.CodeBind)
    {
        Options::OverlayKeyBind = m_overlayKeyBindInfo.Apply();
        VKBindings::Save(); // also save bindings in this case!
    }

    Options::PatchEnableDebug = m_patchEnableDebug;
    Options::PatchRemovePedestrians = m_patchRemovePedestrians;
    Options::PatchAsyncCompute = m_patchAsyncCompute;
    Options::PatchAntialiasing = m_patchAntialiasing;
    Options::PatchSkipStartMenu = m_patchSkipStartMenu;
    Options::PatchDisableIntroMovies = m_patchDisableIntroMovies;
    Options::PatchDisableVignette = m_patchDisableVignette;
    Options::PatchDisableBoundaryTeleport = m_patchDisableBoundaryTeleport;
    Options::PatchDisableWin7Vsync = m_patchDisableWin7Vsync;
    Options::DumpGameOptions = m_dumpGameOptions;

    Options::Save();
}

void Settings::ResetToDefaults()
{
    Options::ResetToDefaults();
    Load();
}
