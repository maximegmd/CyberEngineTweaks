#pragma once

#include "Widget.h"

struct LuaVM;

struct GameLog : Widget
{
    GameLog();
    ~GameLog() override = default;

    WidgetResult OnEnable() override;
    WidgetResult OnDisable() override;
    void Update() override;

    void Log(const std::string& acpText);

private:
    std::recursive_mutex m_gamelogLock{ };
    TiltedPhoques::Vector<std::pair<char, std::string>> m_gamelogLines{ };
    bool m_gamelogShouldScroll{ true };
    bool m_gamelogScroll{ false };
};