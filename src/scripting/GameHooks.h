#pragma once
#include "RED4ext/Api/Sdk.hpp"

struct GameMainThread
{
    static void Create(RED4ext::PluginHandle aHandle, const RED4ext::Sdk* aSdk);
    static GameMainThread& Get();

    void AddBaseInitializationTask(const std::function<bool()>& aFunction);
    void AddInitializationTask(const std::function<bool()>& aFunction);
    void AddRunningTask(const std::function<bool()>& aFunction);
    void AddShutdownTask(const std::function<bool()>& aFunction);
    void AddGenericTask(const std::function<bool()>& aFunction);

private:
    GameMainThread(RED4ext::PluginHandle aHandle, const RED4ext::Sdk* sdk);

    using TStateTick = bool(RED4ext::IGameState*, RED4ext::CGameApplication*);

    static bool HookBaseInitStateTick(RED4ext::CGameApplication* apGameApplication);
    static bool HookInitStateTick(RED4ext::CGameApplication* apGameApplication);
    static bool HookRunningStateTick(RED4ext::CGameApplication* apGameApplication);
    static bool HookShutdownStateTick(RED4ext::CGameApplication* apGameApplication);

    // helper task queue which executes added tasks each drain until they are finished
    struct RepeatedTaskQueue
    {
        void AddTask(const std::function<bool()>& aFunction);
        void Drain();

    private:
        std::recursive_mutex m_mutex;
        TiltedPhoques::Vector<std::function<bool()>> m_tasks;
    };

    struct StateTickOverride
    {
        StateTickOverride(const char* acpRealFunctionName);
        ~StateTickOverride();

        bool OnTick(RED4ext::IGameState*, RED4ext::CGameApplication*);

        RepeatedTaskQueue Tasks;
    };

    std::array<StateTickOverride, 4> m_stateTickOverrides{
        StateTickOverride("CBaseInitializationState::OnTick"),
        StateTickOverride("CInitializationState::OnTick"),
        StateTickOverride( "CRunningState::OnTick"),
        StateTickOverride("CShutdownState::OnTick")};

    RepeatedTaskQueue m_genericQueue;
};
