#pragma once

#include "Widget.h"

namespace HelperWidgets
{
WidgetID ToolbarWidget();

bool BoolWidget(const std::string& aLabel, bool& aCurrent, bool aSaved, float aOffsetX = 0.0f);

enum class THWUCPResult
{
    CHANGED,
    APPLY,
    DISCARD,
    CANCEL
};

THWUCPResult UnsavedChangesPopup(bool& aFirstTime, bool aMadeChanges, TWidgetCB aSaveCB, TWidgetCB aLoadCB, TWidgetCB aCancelCB = nullptr);
}