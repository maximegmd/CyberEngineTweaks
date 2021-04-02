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
        if (ImGui::Button("Bindings"))
            activeID = WidgetID::BINDINGS;
        ImGui::SameLine();
        if (ImGui::Button("Settings"))
            activeID = WidgetID::SETTINGS;
        ImGui::SameLine();
        if (ImGui::Button("TweakDB Editor"))
            activeID = WidgetID::TWEAKDB;
        ImGui::Spacing();
        return activeID;
    }

    bool BindWidget(VKBindInfo& aVKBindInfo, bool aUnbindable, float offset_x)
    {
        VKBindings& vkb { CET::Get().GetBindings() };

        if (aVKBindInfo.IsBinding && !vkb.IsRecordingBind())
        {
            aVKBindInfo.CodeBind = vkb.GetLastRecordingResult();
            aVKBindInfo.IsBinding = false;
        }

        ImVec4 curTextColor { ImGui::GetStyleColorVec4(ImGuiCol_Text) };
        if (aVKBindInfo.CodeBind == 0)
            curTextColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        if (aVKBindInfo.CodeBind != aVKBindInfo.SavedCodeBind)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

        std::string label { aVKBindInfo.Bind.Description + ':' };
        if (aVKBindInfo.Bind.IsHotkey())
            label.insert(0, "[HK] "); // insert [HK] prefix for hotkeys so user knows this input can be assigned up to 4-key combo
        
        ImGui::AlignTextToFramePadding();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset_x);

        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::PushID(&aVKBindInfo.Bind.Description); // ensure we have unique ID by using pointer to Description, is OK, pointer will not be used inside ImGui :P
        ImGui::TextUnformatted(label.c_str());
        ImGui::PopID();
        ImGui::PopStyleColor();
        
        std::string vkStr { (aVKBindInfo.IsBinding) ? ("BINDING...") : (VKBindings::GetBindString(aVKBindInfo.CodeBind)) };
        
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
        
        if (aUnbindable && aVKBindInfo.CodeBind)
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

        return (aVKBindInfo.CodeBind != aVKBindInfo.SavedCodeBind);
    }

    bool BoolWidget(const std::string& aLabel, bool& aCurrent, bool aSaved, float aOffsetX)
    {
        ImVec4 curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
        if (aCurrent != aSaved)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

        ImGui::AlignTextToFramePadding();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + aOffsetX);

        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::TextUnformatted(aLabel.c_str());
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::Checkbox(("##" + aLabel).c_str(), &aCurrent);

        return (aCurrent != aSaved);
    }

    int32_t UnsavedChangesPopup(bool& aFirstTime, bool aMadeChanges, TUCHPSave aSaveCB, TUCHPLoad aLoadCB)
    {
        if (aMadeChanges)
        {
            int32_t res = 0;
            if (aFirstTime)
            {
                ImGui::OpenPopup("Unsaved changes");
                aFirstTime = false;
            }

            if (ImGui::BeginPopupModal("Unsaved changes", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                const auto shorterTextSz { ImGui::CalcTextSize("You have some unsaved changes.").x };
                const auto longerTextSz { ImGui::CalcTextSize("Do you wish to apply them or discard them?").x };
                const auto diffTextSz { longerTextSz - shorterTextSz };

                ImGui::SetCursorPosX(diffTextSz / 2);
                ImGui::TextUnformatted("You have some unsaved changes.");
                ImGui::TextUnformatted("Do you wish to apply them or discard them?");
                ImGui::Separator();

                const auto buttonWidth { (longerTextSz - ImGui::GetStyle().ItemSpacing.x) / 2 };

                if (ImGui::Button("Apply", ImVec2(buttonWidth, 0)))
                {
                    aSaveCB();
                    res = 1;
                    aFirstTime = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Discard", ImVec2(buttonWidth, 0)))
                {
                    aLoadCB();
                    res = -1;
                    aFirstTime = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();

                ImGui::EndPopup();
            }
            return res;
        }
        return 1; // no changes, same as if we were to Apply
    }
}
