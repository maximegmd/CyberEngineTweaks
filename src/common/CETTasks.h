#pragma once

struct CETTasks
{
    CETTasks();
    ~CETTasks();

private:

    std::atomic<bool> m_running;
};