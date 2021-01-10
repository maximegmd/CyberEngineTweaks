#pragma once

#include "ToolbarWidget.h"

struct ModWidgets : public ToolbarWidget
{
    ModWidgets() = default;
    ~ModWidgets() override = default;

    void OnEnable() override;
    void OnDisable() override;
    void Update() override;
};
