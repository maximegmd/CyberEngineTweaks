#pragma once

enum class WidgetResult
{
    DISABLED,
    ENABLED,
    CANCEL
};

struct Widget
{
    Widget(const std::string& acpName, bool aOwnerDraw = false);
    virtual ~Widget() = default;

    virtual WidgetResult OnEnable();
    virtual WidgetResult OnDisable();

    bool IsEnabled() const;
    void Toggle();

    void Draw();

protected:
    virtual void OnUpdate() = 0;

    virtual WidgetResult OnPopup();
    virtual void OnToggle();

    std::string m_name;
    bool m_ownerDraw{false};
    bool m_enabled{false};
    bool m_toggle{false};
    bool m_drawPopup{false};
};

using TWidgetCB = std::function<void()>;

enum class TChangedCBResult
{
    CHANGED,
    APPLY,
    DISCARD,
    CANCEL
};
