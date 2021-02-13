#pragma once

struct Options;
struct CETTasks
{
    CETTasks(Options& aOptions);
    ~CETTasks();

private:

    std::atomic<bool> m_running;
};