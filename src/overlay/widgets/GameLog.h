#pragma once

#include "Widget.h"
#include "LogWindow.h"

struct GameLog : Widget
{
    GameLog();
    ~GameLog() override = default;

protected:
    void OnUpdate() override;

private:
    LogWindow m_logWindow;
};
