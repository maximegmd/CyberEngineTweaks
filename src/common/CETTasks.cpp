#include <stdafx.h>
#include <curl/curl.h>

#include "CETTasks.h"

CETTasks::CETTasks(Options& aOptions)
    : m_running(true)
{
}

CETTasks::~CETTasks()
{
    m_running.store(false);
}
