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
    m_isDirty = false;
}

void Settings::Update()
{
    if (m_wasBindingKey && !m_bindingKey && Options::IsFirstLaunch)
    {
        m_isDirty = true;
        Save();
        Load();
    }

    if (ImGui::Button("Load"))
        Load();
    ImGui::SameLine();
    if (ImGui::Button("Save"))
        Save();
    ImGui::SameLine();
    if (ImGui::Button("Reset to defaults"))
        if (m_isDirty)
            ResetToDefaults();

    ImVec4 curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (m_toolbarKey == 0)
        curTextColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    if (m_toolbarKey != Options::ToolbarKey)
        curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
    ImGui::Text("Toolbar toggle key:");
    ImGui::PopStyleColor();
    ImGui::SameLine();

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
            else if (m_toolbarKey >= VK_NUMPAD0 && m_toolbarKey <= VK_NUMPAD9)
            {
                strncpy(vkChar, "Numpad ", std::size(vkChar));
                char* next = vkChar + std::size("Numpad ")-1;
                *next = '0' + m_toolbarKey & 0xF;
            }
            else if (m_toolbarKey >= VK_F1 && m_toolbarKey <= VK_F24)
            {
                vkChar[0] = 'F';
                int vkKeyFiltered = m_toolbarKey & 0x8F + 1;
                if (vkKeyFiltered & 0x80)
                    vkKeyFiltered = 0x10 | vkKeyFiltered & 0xF;
                if (vkKeyFiltered >= 20)
                {
                    vkChar[1] = '2';
                    vkChar[2] = '0' + vkKeyFiltered - 20;
                }
                else if (vkKeyFiltered >= 10)
                {
                    vkChar[1] = '1';
                    vkChar[2] = '0' + vkKeyFiltered - 10;
                }
                else
                    vkChar[1] = '0' + vkKeyFiltered;
            }
            else if (m_toolbarChar)
                vkChar[0] = m_toolbarChar;
            else
                strncpy(vkChar, "UNKNOWN", std::size(vkChar));


        }
    }
    else
      strncpy(vkChar, "BINDING...", std::size(vkChar));

    if (ImGui::Button(vkChar))
    {
        if (!m_wasBindingKey)
            m_bindingKey = true;
    }
    
    /*m_patchEnableDebug = Options::PatchEnableDebug;
    m_patchRemovePedestrians = Options::PatchRemovePedestrians;
    m_patchAsyncCompute = Options::PatchAsyncCompute;
    m_patchAntialiasing = Options::PatchAntialiasing;
    m_patchSkipStartMenu = Options::PatchSkipStartMenu;
    m_patchDisableIntroMovies = Options::PatchDisableIntroMovies;
    m_patchDisableVignette = Options::PatchDisableVignette;
    m_patchDisableBoundaryTeleport = Options::PatchDisableBoundaryTeleport;
    m_patchDisableWin7Vsync = Options::PatchDisableWin7Vsync;
    m_dumpGameOptions = Options::DumpGameOptions;*/
    
    m_wasBindingKey = m_bindingKey;
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

    m_isDirty = false;
}

void Settings::Save()
{
    Options::ToolbarKey = m_toolbarKey;
    Options::PatchEnableDebug = m_toolbarKey;
    Options::PatchRemovePedestrians = m_toolbarKey;
    Options::PatchAsyncCompute = m_toolbarKey;
    Options::PatchAntialiasing = m_toolbarKey;
    Options::PatchSkipStartMenu = m_toolbarKey;
    Options::PatchDisableIntroMovies = m_toolbarKey;
    Options::PatchDisableVignette = m_toolbarKey;
    Options::PatchDisableBoundaryTeleport = m_toolbarKey;
    Options::PatchDisableWin7Vsync = m_toolbarKey;
    Options::DumpGameOptions = m_toolbarKey;

    Options::Save();

    m_isDirty = false;
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
        case VK_SNAPSHOT:
            return "Print Screen";
        case VK_INSERT:
            return "Insert";
        case VK_DELETE:
            return "Delete";
        case VK_HELP:
            return "Help";

        default:
            return nullptr;
    }
}
