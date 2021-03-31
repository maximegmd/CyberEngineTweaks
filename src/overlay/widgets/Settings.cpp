#include <stdafx.h>

#include "Settings.h"
#include <scripting/LuaVM.h>

Settings::Settings(Options& aOptions, LuaVM& aVm)
    : m_options(aOptions)
    , m_vm(aVm)
{
}

bool Settings::OnEnable()
{
    if (!m_enabled)
    {
        Load();
        m_enabled = true;
    }
    return m_enabled;
}

bool Settings::OnDisable()
{
    if (m_enabled)
    {
        m_vm.BlockUpdate(m_madeChanges);
        m_madeChanges = (HelperWidgets::UnsavedChangesPopup(m_openChangesModal, m_madeChanges, m_saveCB, m_loadCB) == 0);
        m_vm.BlockUpdate(m_madeChanges);
        m_enabled = m_madeChanges;
    }
    return !m_enabled;
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
    
    if (ImGui::BeginTabBar("##SETTINGS", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip))
    {
        if (ImGui::BeginTabItem("Patches"))
        {
            if (ImGui::BeginChild("##SETTINGS_PATCHES"))
            {                
                m_patchesChanged = HelperWidgets::BoolWidget("AMD SMT Patch:", m_patchAmdSmt, m_options.PatchAmdSmt);
                m_patchesChanged |= HelperWidgets::BoolWidget("Remove Pedestrians:", m_patchRemovePedestrians, m_options.PatchRemovePedestrians);
                m_patchesChanged |= HelperWidgets::BoolWidget("Disable Async Compute:", m_patchAsyncCompute, m_options.PatchAsyncCompute);
                m_patchesChanged |= HelperWidgets::BoolWidget("Disable Antialiasing:", m_patchAntialiasing, m_options.PatchAntialiasing);
                m_patchesChanged |= HelperWidgets::BoolWidget("Skip Start Menu:", m_patchSkipStartMenu, m_options.PatchSkipStartMenu);
                m_patchesChanged |= HelperWidgets::BoolWidget("Suppress Intro Movies:", m_patchDisableIntroMovies, m_options.PatchDisableIntroMovies);
                m_patchesChanged |= HelperWidgets::BoolWidget("Disable Vignette:", m_patchDisableVignette, m_options.PatchDisableVignette);
                m_patchesChanged |= HelperWidgets::BoolWidget("Disable Boundary Teleport:", m_patchDisableBoundaryTeleport, m_options.PatchDisableBoundaryTeleport);
                m_patchesChanged |= HelperWidgets::BoolWidget("Disable V-Sync (Windows 7 only):", m_patchDisableWin7Vsync, m_options.PatchDisableWin7Vsync);
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }
        
        if (ImGui::BeginTabItem("Dev"))
        {
            if (ImGui::BeginChild("##SETTINGS_DEV"))
            {

                HelperWidgets::BoolWidget("Draw ImGui Diagnostic Window:", m_options.DrawImGuiDiagnosticWindow, m_options.DrawImGuiDiagnosticWindow);

                m_devChanged  = HelperWidgets::BoolWidget("Remove Dead Bindings:", m_removeDeadBindings, m_options.RemoveDeadBindings);
                m_devChanged |= HelperWidgets::BoolWidget("Enable ImGui Assertions:", m_enableImGuiAssertions, m_options.EnableImGuiAssertions);
                m_devChanged |= HelperWidgets::BoolWidget("Enable Debug Menu:", m_patchEnableDebug, m_options.PatchEnableDebug);
                m_devChanged |= HelperWidgets::BoolWidget("Dump Game Options:", m_dumpGameOptions, m_options.DumpGameOptions);
            }
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        m_madeChanges = m_patchesChanged || m_devChanged;

        ImGui::EndTabBar();
    }
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
