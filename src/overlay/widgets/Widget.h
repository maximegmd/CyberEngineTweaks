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

    virtual bool OnEnable() = 0;
    virtual bool OnDisable() = 0;
    virtual void Update() = 0;
};
