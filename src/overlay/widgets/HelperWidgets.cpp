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

    int32_t BindWidget(const VKBindInfo& acVKBindInfo, float aOffsetX)
    {
        const bool bound = acVKBindInfo.CodeBind != 0;
        const bool modified = acVKBindInfo.CodeBind != acVKBindInfo.SavedCodeBind;

        ImVec4 curTextColor { ImGui::GetStyleColorVec4(ImGuiCol_Text) };
        if (!bound)
            curTextColor = ImVec4(1.0f, modified ? 0.5f : 0.0f, 0.0f, 1.0f);
        else if (modified)
            curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

        std::string label { acVKBindInfo.Bind.get().IsHotkey() ? "[Hotkey] " : "[Input] " };
        label += acVKBindInfo.Bind.get().DisplayName;
        label += ':';

        ImGui::AlignTextToFramePadding();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + aOffsetX);

        ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);
        ImGui::TextUnformatted(label.c_str());
        ImGui::PopStyleColor();

        const std::string vkStr { acVKBindInfo.IsBinding ? "BINDING..." : VKBindings::GetBindString(acVKBindInfo.CodeBind) };

        int32_t ret = 0;

        ImGui::SameLine();
        ImGui::PushID(&acVKBindInfo.CodeBind);
        if (ImGui::Button(vkStr.c_str()))
        {
            if (!acVKBindInfo.IsBinding && !CET::Get().GetBindings().IsRecordingBind())
                ret = 1; // start recording
        }
        ImGui::PopID();

        if (acVKBindInfo.IsUnbindable && acVKBindInfo.CodeBind)
        {
            ImGui::SameLine();
            ImGui::PushID(&acVKBindInfo.SavedCodeBind);
            if (ImGui::Button("UNBIND"))
                ret = -1; // unbind and stop recording if some is in progress
            ImGui::PopID();
        }

        return ret;
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
