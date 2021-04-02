#pragma once

#include "Widget.h"

namespace HelperWidgets
{
WidgetID ToolbarWidget();
void BindWidget(VKBindInfo& aVKBindInfo, const std::string& acId);
bool BoolWidget(const std::string& label, bool& current, bool saved, float offset_x = 0.0f);

using TUCHPSave = std::function<void()>;
using TUCHPLoad = std::function<void()>;
int32_t UnsavedChangesPopup(bool& aFirstTime, bool aMadeChanges, TUCHPSave aSaveCB, TUCHPLoad aLoadCB);
}