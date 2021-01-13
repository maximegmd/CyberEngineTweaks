#pragma once

#include "Widget.h"

struct ModWidgets : public Widget
{
    ModWidgets() = default;
    ~ModWidgets() override = default;

    void OnEnable() override;
    void OnDisable() override;
    void Update() override;
};
