#pragma once

#include "Widget.h"
#include "LogWindow.h"

struct D3D12;
struct GameLog : Widget
{
    GameLog(D3D12& aD3D12);
    ~GameLog() override = default;

protected:
    void OnUpdate() override;

private:
    LogWindow m_logWindow;
};
