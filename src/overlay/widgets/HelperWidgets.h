#pragma once

#include "Widget.h"

namespace HelperWidgets
{
WidgetID ToolbarWidget();
bool BindWidget(VKBindInfo& aVKBindInfo, const std::string& acIdOverlay);
bool BoolWidget(const std::string& label, bool& current, bool saved);
}