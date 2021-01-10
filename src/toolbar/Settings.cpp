#include <stdafx.h>

#include "Settings.h"

void Settings::OnEnable()
{
    Load();
    
    m_wasBindingKey = false;
    m_bindingKey = false;
}

void Settings::OnDisable()
{
    m_wasBindingKey = false;
    m_bindingKey = false;
}

void Settings::Update()
{
    if (m_wasBindingKey && !m_bindingKey && Options::IsFirstLaunch)
    {
        Save();
        Load();
    }

    if (ImGui::Button("Load"))
        Load();
    ImGui::SameLine();
    if (ImGui::Button("Save"))
        Save();
    ImGui::SameLine();
    if (ImGui::Button("Defaults"))
        ResetToDefaults();

    ImGui::Spacing();

    ImVec4 curTextColor;

    // Toolbar Key
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_toolbarKey == 0)
            curTextColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        if (m_toolbarKey != Options::ToolbarKey)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Toolbar Key:");
        ImGui::PopStyleColor();

        char vkChar[64] = { 0 };
        if (!m_bindingKey)
        {
            if (m_toolbarKey == 0)
                strncpy(vkChar, "NOT BOUND", std::size(vkChar));
            else
            {
                const auto* specialName = GetSpecialKeyName(m_toolbarKey);
                if (specialName)
                    strncpy(vkChar, specialName, std::size(vkChar));
                else if (m_toolbarChar)
                    vkChar[0] = m_toolbarChar;
                else
                    strncpy(vkChar, "UNKNOWN", std::size(vkChar));
            }
        }
        else
          strncpy(vkChar, "BINDING...", std::size(vkChar));
        
        ImGui::SameLine();
        if (ImGui::Button(vkChar))
        {
            if (!m_wasBindingKey)
                m_bindingKey = true;
        }
    
        m_wasBindingKey = m_bindingKey;
    }

    // Enable Debug Menu
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_patchEnableDebug != Options::PatchEnableDebug)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Enable Debug Menu:");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox("##PatchEnableDebug", &m_patchEnableDebug);
    }
    
    // Remove Pedestrians
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_patchRemovePedestrians != Options::PatchRemovePedestrians)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Remove Pedestrians:");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox("##PatchRemovePedestrians", &m_patchRemovePedestrians);
    }
    
    // Disable Async Compute
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_patchAsyncCompute != Options::PatchAsyncCompute)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Disable Async Compute:");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox("##PatchAsyncCompute", &m_patchAsyncCompute);
    }
    
    // Disable Antialiasing
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_patchAntialiasing != Options::PatchAntialiasing)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Disable Antialiasing:");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox("##PatchAntialiasing", &m_patchAntialiasing);
    }

    // Skip Start Menu
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_patchSkipStartMenu != Options::PatchSkipStartMenu)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Skip Start Menu:");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox("##PatchSkipStartMenu", &m_patchSkipStartMenu);
    }
    
    // Suppress Intro Movies
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_patchDisableIntroMovies != Options::PatchDisableIntroMovies)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Suppress Intro Movies:");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox("##PatchDisableIntroMovies", &m_patchDisableIntroMovies);
    }
    
    // Disable Vignette
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_patchDisableVignette != Options::PatchDisableVignette)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Disable Vignette:");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox("##PatchDisableVignette", &m_patchDisableVignette);
    }
    
    // Disable Boundary Teleport
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_patchDisableBoundaryTeleport != Options::PatchDisableBoundaryTeleport)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Disable Boundary Teleport:");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox("##PatchDisableBoundaryTeleport", &m_patchDisableBoundaryTeleport);
    }
    
    // Disable V-Sync (Windows 7 only)
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_patchDisableWin7Vsync != Options::PatchDisableWin7Vsync)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Disable V-Sync (Windows 7 only):");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox("##PatchDisableWin7Vsync", &m_patchDisableWin7Vsync);
    }
    
    // Dump Game Options
    {
        curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (m_dumpGameOptions != Options::DumpGameOptions)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::Text("Dump Game Options:");
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox("##DumpGameOptions", &m_dumpGameOptions);
    }
}

bool Settings::IsBindingKey() const
{
    return m_wasBindingKey;
}

void Settings::RecordKeyDown(UINT aVKCode)
{
    m_toolbarKey = aVKCode;
    m_toolbarChar = MapVirtualKey(aVKCode, MAPVK_VK_TO_CHAR);
}

void Settings::RecordKeyUp(UINT aVKCode)
{
    if (m_toolbarKey == aVKCode)
        m_bindingKey = false;
}

void Settings::Load()
{
    Options::Load();

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

void Settings::Save()
{
    Options::ToolbarKey = m_toolbarKey;
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

const char* Settings::GetSpecialKeyName(UINT aVKCode)
{
    switch (aVKCode)
    {
        case VK_LBUTTON:
            return "Mouse LB";
        case VK_RBUTTON:
            return "Mouse RB";
        case VK_MBUTTON:
            return "Mouse MB";
        case VK_XBUTTON1:
            return "Mouse X1";
        case VK_XBUTTON2:
            return "Mouse X2";
        case VK_BACK:
            return "Backspace";
        case VK_TAB:
            return "Tab";
        case VK_CLEAR:
            return "Clear";
        case VK_RETURN:
            return "Enter";
        case VK_SHIFT:
            return "Shift";
        case VK_CONTROL:
            return "Ctrl";
        case VK_MENU:
            return "Alt";
        case VK_PAUSE:
            return "Pause";
        case VK_CAPITAL:
            return "Caps Lock";
        case VK_ESCAPE:
            return "Esc";
        case VK_SPACE:
            return "Space";
        case VK_PRIOR:
            return "Page Up";
        case VK_NEXT:
            return "Page Down";
        case VK_END:
            return "End";
        case VK_HOME:
            return "Home";
        case VK_LEFT:
            return "Left Arrow";
        case VK_UP:
            return "Up Arrow";
        case VK_RIGHT:
            return "Right Arrow";
        case VK_DOWN:
            return "Down Arrow";
        case VK_SELECT:
            return "Select";
        case VK_PRINT:
            return "Print";
        case VK_EXECUTE:
            return "Execute";
        case VK_INSERT:
            return "Insert";
        case VK_DELETE:
            return "Delete";
        case VK_HELP:
            return "Help";
        case VK_NUMPAD0:
            return "Numpad 0";
        case VK_NUMPAD1:
            return "Numpad 1";
        case VK_NUMPAD2:
            return "Numpad 2";
        case VK_NUMPAD3:
            return "Numpad 3";
        case VK_NUMPAD4:
            return "Numpad 4";
        case VK_NUMPAD5:
            return "Numpad 5";
        case VK_NUMPAD6:
            return "Numpad 6";
        case VK_NUMPAD7:
            return "Numpad 7";
        case VK_NUMPAD8:
            return "Numpad 8";
        case VK_NUMPAD9:
            return "Numpad 9";
        case VK_F1:
            return "F1";
        case VK_F2:
            return "F2";
        case VK_F3:
            return "F3";
        case VK_F4:
            return "F4";
        case VK_F5:
            return "F5";
        case VK_F6:
            return "F6";
        case VK_F7:
            return "F7";
        case VK_F8:
            return "F8";
        case VK_F9:
            return "F9";
        case VK_F10:
            return "F10";
        case VK_F11:
            return "F11";
        case VK_F12:
            return "F12";
        case VK_NUMLOCK:
            return "Num Lock";
        case VK_SCROLL:
            return "Scroll Lock";
        default:
            return nullptr;
    }
}
