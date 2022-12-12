#pragma once

#if GAME_CYBERPUNK
using IGameState = RED4ext::IGameState;
using CGameApplication = RED4ext::CGameApplication;
#else
struct IGameState
{
    virtual const char* GetName() = 0;                 // 00
    virtual ~IGameState() = 0;                         // 08
    virtual bool OnEnter(CGameApplication* aApp) = 0;  // 10
    virtual bool OnUpdate(CGameApplication* aApp) = 0; // 18
    virtual bool OnExit(CGameApplication* aApp) = 0;   // 20
};
struct CGameApplication;
#endif

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

    using TStateTick = bool(IGameState*, CGameApplication*);

    static bool HookStateTick(IGameState* apThisState, CGameApplication* apGameApplication);

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
        StateTickOverride(const uintptr_t acOffset, const char* acpRealFunctionName);
        ~StateTickOverride();

        bool OnTick(IGameState*, CGameApplication*);

        uint8_t* Location = nullptr;
        TStateTick* RealFunction = nullptr;
        RepeatedTaskQueue Tasks;
    };

    std::array<StateTickOverride, 4> m_stateTickOverrides{
        StateTickOverride(Game::Addresses::CBaseInitializationState_OnTick, "CBaseInitializationState::OnTick"),
        StateTickOverride(Game::Addresses::CInitializationState_OnTick, "CInitializationState::OnTick"),
        StateTickOverride(Game::Addresses::CRunningState_OnTick, "CRunningState::OnTick"),
        StateTickOverride(Game::Addresses::CShutdownState_OnTick, "CShutdownState::OnTick")};

    RepeatedTaskQueue m_genericQueue;
};
