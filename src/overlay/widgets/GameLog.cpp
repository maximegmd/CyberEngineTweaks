#include "stdafx.h"

#include "GameLog.h"

GameLog::GameLog()
    : Widget("Game Log")
    , m_logWindow("gamelog")
{
}

void GameLog::OnUpdate()
{
    m_logWindow.Draw({-FLT_MIN, -FLT_MIN});
}
