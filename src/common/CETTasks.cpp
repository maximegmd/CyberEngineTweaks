#include "stdafx.h"

#include "CETTasks.h"

CETTasks::CETTasks()
    : m_running(true)
{
}

CETTasks::~CETTasks()
{
    m_running.store(false);
}
