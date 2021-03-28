#pragma once

enum class WidgetID
{
    CONSOLE,
    BINDINGS,
    SETTINGS,
    TWEAKDB,
    COUNT
};

struct Widget
{
    virtual ~Widget() = default;

    virtual void OnEnable() = 0;
    virtual void OnDisable() = 0;
    virtual void Update() = 0;
};
