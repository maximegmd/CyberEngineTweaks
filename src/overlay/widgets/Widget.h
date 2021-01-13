#pragma once

enum WidgetID
{
    MODS,
    CONSOLE,
    HOTKEYS,
    SETTINGS,
    COUNT
};

struct Widget
{
    virtual ~Widget() = default;

    virtual void OnEnable() = 0;
    virtual void OnDisable() = 0;
    virtual void Update() = 0;
};
