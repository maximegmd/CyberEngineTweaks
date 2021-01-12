#pragma once

inline const char* GetSpecialKeyName(UINT aVKCode)
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

inline void BindWidget(VKBindInfo& aVKBindInfo)
{
    if (aVKBindInfo.IsBinding && !VKBindings::IsRecordingBind())
    {
        aVKBindInfo.CodeBind = VKBindings::GetLastRecordingResult();
        aVKBindInfo.IsBinding = false;
    }

    ImVec4 curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (aVKBindInfo.CodeBind == 0)
        curTextColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    if (aVKBindInfo.CodeBind != aVKBindInfo.SavedCodeBind)
        curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    std::string label = aVKBindInfo.Bind.Description + ':';
    ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
    ImGui::PushID(&aVKBindInfo.Bind.Description);
    ImGui::Text(label.c_str());
    ImGui::PopID();
    ImGui::PopStyleColor();
    
    std::string vkStr = { };
    if (!aVKBindInfo.IsBinding)
    {
        if (aVKBindInfo.CodeBind == 0)
            vkStr = "NOT BOUND";
        else
        {
            auto decoded = VKBindings::DecodeVKCodeBind(aVKBindInfo.CodeBind);
            for (auto vkCode : decoded)
            {
                if (vkCode == 0)
                    break;

                const auto* specialName = GetSpecialKeyName(vkCode);
                if (specialName)
                    vkStr += specialName;
                else
                {
                    char vkChar = MapVirtualKey(vkCode, MAPVK_VK_TO_CHAR);
                    if (vkChar != 0)
                        vkStr += vkChar;
                    else
                        vkStr += "UNKNOWN";
                }
                vkStr += " + ";
            }
            vkStr.erase(vkStr.rfind(" + "));
        }
    }
    else
        vkStr = "BINDING...";
    
    ImGui::SameLine();
    ImGui::PushID(&aVKBindInfo.Bind.ID);
    if (ImGui::Button(vkStr.c_str()))
    {
        if (!aVKBindInfo.IsBinding)
        {
            VKBindings::StartRecordingBind(aVKBindInfo.Bind);
            aVKBindInfo.IsBinding = true;
        }
    }
    ImGui::PopID();
}

inline void BoolWidget(const std::string& label, bool& current, bool saved)
{
    ImVec4 curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (current != saved)
        curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
    ImGui::Text(label.c_str());
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::Checkbox(("##" + label).c_str(), &current);
}