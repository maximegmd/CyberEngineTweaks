#pragma once

#include "Widget.h"

struct D3D12;
struct ImGuiDebug : Widget
{
    ImGuiDebug();
    ~ImGuiDebug() override = default;

protected:
    void OnUpdate() override;
};
