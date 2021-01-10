#include <stdafx.h>

#include "Settings.h"

void Settings::OnEnable()
{
    m_toolbarKey = Options::ToolbarKey;
    m_toolbarChar = Options::ToolbarChar;
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

void Settings::OnDisable()
{
    
}

void Settings::Update()
{
    ImGui::Text("Settings widget here!");
    if (ImGui::Button("Load"))
    {
        Options::Load();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        Options::Save();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset to defaults"))
    {
        Options::ResetToDefaults();
    }
    
    m_toolbarKey = Options::ToolbarKey;
    m_toolbarChar = Options::ToolbarChar;
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
