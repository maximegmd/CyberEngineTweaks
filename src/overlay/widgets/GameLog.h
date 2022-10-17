#pragma once

#include "Widget.h"

struct D3D12;
struct GameLog : Widget
{
    GameLog(D3D12& aD3D12);
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
    D3D12& m_d3d12;
};