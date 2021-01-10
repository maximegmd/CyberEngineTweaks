#pragma once

struct ToolbarWidget
{
    virtual ~ToolbarWidget() = default;

    virtual void OnEnable() = 0;
    virtual void OnDisable() = 0;
    virtual void Update() = 0;
};
