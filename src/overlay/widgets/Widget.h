#pragma once

enum class WidgetID
{
    CONSOLE,
    BINDINGS,
    SETTINGS,
    TWEAKDB,
    GAMELOG,
    COUNT
};

enum class WidgetResult
{
    DISABLED,
    ENABLED,
    CANCEL
};

struct Widget
{
    virtual ~Widget() = default;

    virtual WidgetResult OnEnable() = 0;
    virtual WidgetResult OnDisable() = 0;
    virtual void Update() = 0;
};

using TWidgetCB = std::function<void()>;
