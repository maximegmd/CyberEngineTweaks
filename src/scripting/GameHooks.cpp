#include <stdafx.h>

#include "GameHooks.h"

#include "RED4ext/GameStates.hpp"

static std::unique_ptr<GameMainThread> s_pGameMainThread;

void GameMainThread::RepeatedTaskQueue::AddTask(const std::function<bool()>& aFunction)
{
    std::lock_guard lock(m_mutex);
    m_tasks.emplace_back(aFunction);
}

void GameMainThread::RepeatedTaskQueue::Drain()
{
    std::lock_guard lock(m_mutex);
    for (auto taskIt = m_tasks.begin(); taskIt != m_tasks.end();)
    {
        if ((*taskIt)())
            taskIt = m_tasks.erase(taskIt);
        else
            ++taskIt;
    }
}
GameMainThread::StateTickOverride::StateTickOverride(const uintptr_t acOffset, const char* acpRealFunctionName)
    : Offset(acOffset)
    , RealFunctionName(acpRealFunctionName)
{
}

bool GameMainThread::StateTickOverride::OnTick(RED4ext::IGameState* apThisState, RED4ext::CGameApplication* apGameApplication)
{
    Tasks.Drain();

    return RealFunction(apThisState, apGameApplication);
}

GameMainThread& GameMainThread::Get()
{
    static GameMainThread s_gameMainThread;

    return s_gameMainThread;
}

void GameMainThread::AddBaseInitializationTask(const std::function<bool()>& aFunction)
{
    constexpr auto cStateIndex = static_cast<size_t>(RED4ext::EGameStateType::BaseInitialization);
    return m_stateTickOverrides[cStateIndex].Tasks.AddTask(aFunction);
}

void GameMainThread::AddInitializationTask(const std::function<bool()>& aFunction)
{
    constexpr auto cStateIndex = static_cast<size_t>(RED4ext::EGameStateType::Initialization);
    return m_stateTickOverrides[cStateIndex].Tasks.AddTask(aFunction);
}

void GameMainThread::AddRunningTask(const std::function<bool()>& aFunction)
{
    constexpr auto cStateIndex = static_cast<size_t>(RED4ext::EGameStateType::Running);
    return m_stateTickOverrides[cStateIndex].Tasks.AddTask(aFunction);
}

void GameMainThread::AddShutdownTask(const std::function<bool()>& aFunction)
{
    constexpr auto cStateIndex = static_cast<size_t>(RED4ext::EGameStateType::Shutdown);
    return m_stateTickOverrides[cStateIndex].Tasks.AddTask(aFunction);
}

void GameMainThread::AddGenericTask(const std::function<bool()>& aFunction)
{
    m_genericQueue.AddTask(aFunction);
}

GameMainThread::GameMainThread()
{
    Hook();
}

GameMainThread::~GameMainThread()
{
    Unhook();
}

bool GameMainThread::HookStateTick(RED4ext::IGameState* apThisState, RED4ext::CGameApplication* apGameApplication)
{
    auto& gmt = Get();

    // drain generic tasks
    gmt.m_genericQueue.Drain();

    // execute specific state tasks, including original function
    const auto cStateIndex = static_cast<size_t>(apThisState->GetType());
    return gmt.m_stateTickOverrides[cStateIndex].OnTick(apThisState, apGameApplication);
}

void GameMainThread::Hook()
{
    for (auto& stateTickOverride : m_stateTickOverrides)
    {
        if (!stateTickOverride.Location)
        {
            const RED4ext::RelocPtr<uint8_t> func(stateTickOverride.Offset);
            stateTickOverride.Location = func.GetAddr();
        }
        if (stateTickOverride.Location)
        {
            if (MH_CreateHook(stateTickOverride.Location, reinterpret_cast<void*>(&GameMainThread::HookStateTick), reinterpret_cast<void**>(&stateTickOverride.RealFunction)) != MH_OK ||
                MH_EnableHook(stateTickOverride.Location) != MH_OK)
                Log::Error("Could not hook main thread function {}! Main thread is not completely hooked!", stateTickOverride.RealFunctionName);
            else
                Log::Info("Main thread function {} hook complete!", stateTickOverride.RealFunctionName);
        }
        else
            Log::Error("Could not locate {}! Main thread is not completely hooked!", stateTickOverride.RealFunctionName);
    }
}

void GameMainThread::Unhook()
{
    for (auto& stateTickOverride : m_stateTickOverrides)
    {
        MH_DisableHook(stateTickOverride.Location);

        stateTickOverride.Location = nullptr;
        stateTickOverride.RealFunction = nullptr;
    }
}
