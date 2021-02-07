#pragma once

struct Options;
struct CETTasks
{
    CETTasks(Options& aOptions);
    ~CETTasks();

private:

    void RunTelemetry();
    std::thread m_telemetryThread;
    std::atomic<bool> m_running;
};