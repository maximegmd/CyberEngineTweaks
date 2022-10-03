#include <stdafx.h>

#include "HelperWidgets.h"

namespace HelperWidgets
{

    WidgetID ToolbarWidget()
    {
        const auto itemWidth = GetAlignedItemWidth(static_cast<int64_t>(WidgetID::COUNT));

        WidgetID activeID = WidgetID::COUNT;
        if (ImGui::Button("Console", ImVec2(itemWidth, 0)))
            activeID = WidgetID::CONSOLE;
        ImGui::SameLine();
        if (ImGui::Button("Bindings", ImVec2(itemWidth, 0)))
            activeID = WidgetID::BINDINGS;
        ImGui::SameLine();
        if (ImGui::Button("Settings", ImVec2(itemWidth, 0)))
            activeID = WidgetID::SETTINGS;
        ImGui::SameLine();
        if (ImGui::Button("TweakDB Editor", ImVec2(itemWidth, 0)))
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

                const auto itemWidth = HelperWidgets::GetAlignedItemWidth(3);

                if (ImGui::Button("Apply", ImVec2(itemWidth, 0)))
                {
                    if (aSaveCB)
                        aSaveCB();
                    res = THWUCPResult::APPLY;
                    aFirstTime = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Discard", ImVec2(itemWidth, 0)))
                {
                    if (aLoadCB)
                        aLoadCB();
                    res = THWUCPResult::DISCARD;
                    aFirstTime = true;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(itemWidth, 0)))
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

    float GetAlignedItemWidth(int64_t aItemsCount)
    {
        return (ImGui::GetWindowContentRegionWidth() - static_cast<float>(aItemsCount - 1) * ImGui::GetStyle().ItemSpacing.x) / static_cast<float>(aItemsCount);
    }

}
