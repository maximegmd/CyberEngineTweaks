#include <stdafx.h>

#include "GameLog.h"

GameLog::GameLog(D3D12& aD3D12)
    : Widget("Game Log")
    , m_logWindow(aD3D12, "gamelog")
{
}

void GameLog::OnUpdate()
{
    m_logWindow.Draw({-FLT_MIN, -FLT_MIN});
}
