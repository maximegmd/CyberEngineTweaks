#pragma once

struct GameMainThread
{
    static void Initialize();
    static void Shutdown();

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

    TiltedPhoques::Vector<std::function<bool()>> m_baseInitializationTasks;
    TiltedPhoques::Vector<std::function<bool()>> m_initializationTasks;
    TiltedPhoques::Vector<std::function<bool()>> m_runningTasks;
    TiltedPhoques::Vector<std::function<bool()>> m_shutdownTasks;
    TiltedPhoques::Vector<std::function<bool()>> m_genericTasks;
};
