#pragma once

#include "Widget.h"

namespace HelperWidgets
{
    WidgetID ToolbarWidget();
    void BindWidget(VKBindInfo& aVKBindInfo);
    void BoolWidget(const std::string& label, bool& current, bool saved);
    
}