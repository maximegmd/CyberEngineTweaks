#include <stdafx.h>

#include "HelperWidgets.h"

namespace HelperWidgets
{

    WidgetID ToolbarWidget()
    {
        WidgetID activeID = WidgetID::COUNT;
        if (ImGui::Button("Mod Widgets"))
            activeID = WidgetID::MODS;
        ImGui::SameLine();
        if (ImGui::Button("Console"))
            activeID = WidgetID::CONSOLE;
        ImGui::SameLine();
        if (ImGui::Button("Hotkeys"))
            activeID = WidgetID::HOTKEYS;
        ImGui::SameLine();
        if (ImGui::Button("Settings"))
            activeID = WidgetID::SETTINGS;
        ImGui::Spacing();
        return activeID;
    }

    void BindWidget(VKBindInfo& aVKBindInfo)
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

                    const char* specialName = VKBindings::GetSpecialKeyName(vkCode);
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

    void BoolWidget(const std::string& label, bool& current, bool saved)
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

}
