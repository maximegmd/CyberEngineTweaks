#pragma once

struct GameMainThread
{
    static GameMainThread& Get();

    void AddBaseInitializationTask(const std::function<bool()>& aFunction);
    void AddInitializationTask(const std::function<bool()>& aFunction);
    void AddRunningTask(const std::function<bool()>& aFunction);
    void AddShutdownTask(const std::function<bool()>& aFunction);
    void AddGenericTask(const std::function<bool()>& aFunction);

private:
    GameMainThread() = default;

    using TStateTick = bool(RED4ext::IGameState*, RED4ext::CGameApplication*);

    static bool HookStateTick(RED4ext::IGameState* apThisState, RED4ext::CGameApplication* apGameApplication);

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
        StateTickOverride(uint32_t aHash, const char* acpRealFunctionName);
        ~StateTickOverride();

        bool OnTick(RED4ext::IGameState*, RED4ext::CGameApplication*);

        uint8_t* Location = nullptr;
        TStateTick* RealFunction = nullptr;
        RepeatedTaskQueue Tasks;
    };

    std::array<StateTickOverride, 4> m_stateTickOverrides{
        StateTickOverride(CyberEngineTweaks::AddressHashes::CBaseInitializationState_OnTick, "CBaseInitializationState::OnTick"),
        StateTickOverride(CyberEngineTweaks::AddressHashes::CInitializationState_OnTick, "CInitializationState::OnTick"),
        StateTickOverride(CyberEngineTweaks::AddressHashes::CRunningState_OnTick, "CRunningState::OnTick"),
        StateTickOverride(CyberEngineTweaks::AddressHashes::CShutdownState_OnTick, "CShutdownState::OnTick")};

    RepeatedTaskQueue m_genericQueue;
};
