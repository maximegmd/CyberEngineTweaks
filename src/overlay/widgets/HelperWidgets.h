#pragma once

#include "Widget.h"

namespace HelperWidgets
{
WidgetID ToolbarWidget();
bool BindWidget(VKBindInfo& aVKBindInfo, const std::string& acIdOverlay);
bool BoolWidget(const std::string& label, bool& current, bool saved);

using TUCHPSave = std::function<void()>;
using TUCHPLoad = std::function<void()>;
int32_t UnappliedChangesPopup(bool& aFirstTime, bool aMadeChanges, TUCHPSave aSaveCB, TUCHPLoad aLoadCB);
}