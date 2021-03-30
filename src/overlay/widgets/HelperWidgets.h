#pragma once

#include "Widget.h"

namespace HelperWidgets
{
WidgetID ToolbarWidget();
bool BindWidget(VKBindInfo& aVKBindInfo, bool aUnbindable, float offset_x = 0.0f);
bool BoolWidget(const std::string& label, bool& current, bool saved, float offset_x = 0.0f);

using TUCHPSave = std::function<void()>;
using TUCHPLoad = std::function<void()>;
int32_t UnsavedChangesPopup(bool& aFirstTime, bool aMadeChanges, TUCHPSave aSaveCB, TUCHPLoad aLoadCB);
}