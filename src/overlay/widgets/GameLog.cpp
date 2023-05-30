#include <stdafx.h>

#include "GameLog.h"

#include <CET.h>

GameLog::GameLog()
    : Widget(ICON_MD_FILE_DOCUMENT, _noop("Game Log"))
    , m_logWindow("gamelog")
{
}

void GameLog::OnUpdate()
{
    m_logWindow.Draw({-FLT_MIN, -FLT_MIN});
}
