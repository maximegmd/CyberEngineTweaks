#pragma once

#include "Widget.h"

namespace HelperWidgets
{
WidgetID ToolbarWidget();
void BindWidget(VKBindInfo& aVKBindInfo, const std::string& acId);
void BoolWidget(const std::string& label, bool& current, bool saved);
}