#include <stdafx.h>

#include "Settings.h"

#include "HelperWidgets.h"

#include <overlay/Overlay.h>

Settings::Settings(Overlay& aOverlay, VKBindings& aBindings, Options& aOptions)
    : m_bindings(aBindings)
    , m_overlay(aOverlay)
    , m_options(aOptions)
{
}

void Settings::OnEnable()
{
    Load();
    
    m_bindings.StopRecordingBind();
}

void Settings::OnDisable()
{
    m_bindings.StopRecordingBind();
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

    if (!m_options.IsFirstLaunch)
        ImGui::BeginChild("##SETTINGS_ACTUAL", ImVec2(0,0), true);

    HelperWidgets::BindWidget(m_overlayKeyBindInfo, m_overlay.GetBind().ID);
    if (m_options.IsFirstLaunch && (m_overlayKeyBindInfo.SavedCodeBind != m_overlayKeyBindInfo.CodeBind))
    {
        Save();
        Load();
    }

    HelperWidgets::BoolWidget("Enable Debug Menu:", m_patchEnableDebug, m_options.PatchEnableDebug);
    HelperWidgets::BoolWidget("Remove Pedestrians:", m_patchRemovePedestrians, m_options.PatchRemovePedestrians);
    HelperWidgets::BoolWidget("Disable Async Compute:", m_patchAsyncCompute, m_options.PatchAsyncCompute);
    HelperWidgets::BoolWidget("Disable Antialiasing:", m_patchAntialiasing, m_options.PatchAntialiasing);
    HelperWidgets::BoolWidget("Skip Start Menu:", m_patchSkipStartMenu, m_options.PatchSkipStartMenu);
    HelperWidgets::BoolWidget("Suppress Intro Movies:", m_patchDisableIntroMovies, m_options.PatchDisableIntroMovies);
    HelperWidgets::BoolWidget("Disable Vignette:", m_patchDisableVignette, m_options.PatchDisableVignette);
    HelperWidgets::BoolWidget("Disable Boundary Teleport:", m_patchDisableBoundaryTeleport, m_options.PatchDisableBoundaryTeleport);
    HelperWidgets::BoolWidget("Disable V-Sync (Windows 7 only):", m_patchDisableWin7Vsync, m_options.PatchDisableWin7Vsync);
    HelperWidgets::BoolWidget("Dump Game Options:", m_dumpGameOptions, m_options.DumpGameOptions);

    if (!m_options.IsFirstLaunch)
        ImGui::EndChild();
}

void Settings::Load()
{
    m_options.Load();

    m_overlayKeyBindInfo.Fill(m_options.OverlayKeyBind, m_overlay.GetBind());
    m_patchEnableDebug = m_options.PatchEnableDebug;
    m_patchRemovePedestrians = m_options.PatchRemovePedestrians;
    m_patchAsyncCompute = m_options.PatchAsyncCompute;
    m_patchAntialiasing = m_options.PatchAntialiasing;
    m_patchSkipStartMenu = m_options.PatchSkipStartMenu;
    m_patchDisableIntroMovies = m_options.PatchDisableIntroMovies;
    m_patchDisableVignette = m_options.PatchDisableVignette;
    m_patchDisableBoundaryTeleport = m_options.PatchDisableBoundaryTeleport;
    m_patchDisableWin7Vsync = m_options.PatchDisableWin7Vsync;
    m_dumpGameOptions = m_options.DumpGameOptions;
}

void Settings::Save()
{
    if (m_overlayKeyBindInfo.SavedCodeBind != m_overlayKeyBindInfo.CodeBind)
    {
        m_options.OverlayKeyBind = m_overlayKeyBindInfo.Apply();
        m_bindings.Save(); // also save bindings in this case!
    }

    m_options.PatchEnableDebug = m_patchEnableDebug;
    m_options.PatchRemovePedestrians = m_patchRemovePedestrians;
    m_options.PatchAsyncCompute = m_patchAsyncCompute;
    m_options.PatchAntialiasing = m_patchAntialiasing;
    m_options.PatchSkipStartMenu = m_patchSkipStartMenu;
    m_options.PatchDisableIntroMovies = m_patchDisableIntroMovies;
    m_options.PatchDisableVignette = m_patchDisableVignette;
    m_options.PatchDisableBoundaryTeleport = m_patchDisableBoundaryTeleport;
    m_options.PatchDisableWin7Vsync = m_patchDisableWin7Vsync;
    m_options.DumpGameOptions = m_dumpGameOptions;

    m_options.Save();
}

void Settings::ResetToDefaults()
{
    m_options.ResetToDefaults();
    Load();
}
