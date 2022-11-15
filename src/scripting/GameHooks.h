#pragma once

struct GameMainThread
{
    static GameMainThread& Get();

    void AddBaseInitializationTask(const std::function<bool()>& aFunction);
    void AddInitializationTask(const std::function<bool()>& aFunction);
    void AddRunningTask(const std::function<bool()>& aFunction);
    void AddShutdownTask(const std::function<bool()>& aFunction);
    void AddGenericTask(const std::function<bool()>& aFunction);

    ~GameMainThread();

private:
    GameMainThread();

    void Hook();
    void Unhook() const;

    using TMainThreadStateTick = bool(RED4ext::IGameState*, RED4ext::CGameApplication*);

    static bool HookMainThreadStateTick(RED4ext::IGameState* apThisState, RED4ext::CGameApplication* apGameApplication);

    std::array<std::pair<uint8_t*, TMainThreadStateTick*>, 4> m_ppMainThreadStateTickLocations;

    // helper task queue which executes added tasks each drain until they are finished
    struct TaskQueue
    {
        void AddTask(const std::function<bool()>& aFunction);
        void Drain();

    private:
        std::recursive_mutex m_mutex;
        TiltedPhoques::Vector<std::function<bool()>> m_tasks;
    };

    TaskQueue m_baseInitializationQueue;
    TaskQueue m_initializationQueue;
    TaskQueue m_runningQueue;
    TaskQueue m_shutdownQueue;
    TaskQueue m_genericQueue;
};
