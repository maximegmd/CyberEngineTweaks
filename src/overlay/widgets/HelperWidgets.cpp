#include <stdafx.h>

#include "HelperWidgets.h"

#include "CET.h"
#include "overlay/Overlay.h"

namespace HelperWidgets
{

    WidgetID ToolbarWidget()
    {
        WidgetID activeID = WidgetID::COUNT;
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

    void BindWidget(VKBindInfo& aVKBindInfo, const std::string& acId)
    {
        VKBindings& vkb = CET::Get().GetBindings();

        if (aVKBindInfo.IsBinding && !vkb.IsRecordingBind())
        {
            aVKBindInfo.CodeBind = vkb.GetLastRecordingResult();
            aVKBindInfo.IsBinding = false;
        }

        ImVec4 curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (aVKBindInfo.CodeBind == 0)
            curTextColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        if (aVKBindInfo.CodeBind != aVKBindInfo.SavedCodeBind)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

        const std::string label = aVKBindInfo.Bind.Description + ':';
        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::PushID(&aVKBindInfo.Bind.Description); // ensure we have unique ID by using pointer to Description, is OK, pointer will not be used inside ImGui :P
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
        ImGui::PushID(&aVKBindInfo.Bind.ID[0]); // same as PushID before, just make it pointer to ID and make sure we point to first char (so we can make one more unique ID from this pointer)
        if (ImGui::Button(vkStr.c_str()))
        {
            if (!aVKBindInfo.IsBinding)
            {
                vkb.StartRecordingBind(aVKBindInfo.Bind);
                aVKBindInfo.IsBinding = true;
            }
        }
        ImGui::PopID();
        
        if (aVKBindInfo.CodeBind && (aVKBindInfo.Bind.ID != acId)) // make an exception for Overlay key
        {
            ImGui::PushID(&aVKBindInfo.Bind.ID[1]); // same as PushID before, just make pointer a bit bigger :)
            ImGui::SameLine();
            if (ImGui::Button("UNBIND"))
            {
                if (aVKBindInfo.IsBinding)
                {
                    vkb.StopRecordingBind();
                    aVKBindInfo.IsBinding = false;
                }
                vkb.UnBind(aVKBindInfo.CodeBind);
                aVKBindInfo.CodeBind = 0;
            }
            ImGui::PopID();
        }
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
