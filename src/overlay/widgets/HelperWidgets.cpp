#include <stdafx.h>

#include "HelperWidgets.h"

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

    THWUCPResult UnsavedChangesPopup(bool& aFirstTime, bool aMadeChanges, TWidgetCB aSaveCB, TWidgetCB aLoadCB, TWidgetCB aCancelCB)
    {
        if (aMadeChanges)
        {
            auto res = THWUCPResult::CHANGED;
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

                const auto buttonWidth { (longerTextSz - (2.0f * ImGui::GetStyle().ItemSpacing.x)) / 3.0f };

                if (ImGui::Button("Apply", ImVec2(buttonWidth, 0)))
                {
                    if (aSaveCB)
                        aSaveCB();
                    res = THWUCPResult::APPLY;
                    aFirstTime = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Discard", ImVec2(buttonWidth, 0)))
                {
                    if (aLoadCB)
                        aLoadCB();
                    res = THWUCPResult::DISCARD;
                    aFirstTime = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0)))
                {
                    if (aCancelCB)
                        aCancelCB();
                    res = THWUCPResult::CANCEL;
                    aFirstTime = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();

                ImGui::EndPopup();
            }
            return res;
        }
        return THWUCPResult::APPLY; // no changes, same as if we were to Apply
    }
}
