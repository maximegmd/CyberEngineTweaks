#pragma once

#include "ToolbarWidget.h"

struct Keybinds : public ToolbarWidget
{
    Keybinds() = default;
    ~Keybinds() override = default;

    void OnEnable() override;
    void OnDisable() override;
    void Update() override;
};
