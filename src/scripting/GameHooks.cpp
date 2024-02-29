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
GameMainThread::StateTickOverride::StateTickOverride(const char* acpRealFunctionName)
{
}

GameMainThread::StateTickOverride::~StateTickOverride()
{
}

bool GameMainThread::StateTickOverride::OnTick(RED4ext::IGameState* apThisState, RED4ext::CGameApplication* apGameApplication)
{
    Tasks.Drain();

    return true;
}

static GameMainThread* s_gameMainThread = nullptr;

GameMainThread& GameMainThread::Get()
{
    assert(s_gameMainThread);
    return *s_gameMainThread;
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

void GameMainThread::Create(RED4ext::PluginHandle aHandle, const RED4ext::Sdk* aSdk)
{
    if (!s_gameMainThread)
        s_gameMainThread = new GameMainThread(aHandle, aSdk);
}

GameMainThread::GameMainThread(RED4ext::PluginHandle aHandle, const RED4ext::Sdk* sdk)
{
    {
        RED4ext::GameState state{nullptr, &HookBaseInitStateTick, nullptr};
        sdk->gameStates->Add(aHandle, RED4ext::EGameStateType::BaseInitialization, &state);
    }
    {
        RED4ext::GameState state{nullptr, &HookInitStateTick, nullptr};
        sdk->gameStates->Add(aHandle, RED4ext::EGameStateType::Initialization, &state);
    }
    {
        RED4ext::GameState state{nullptr, &HookRunningStateTick, nullptr};
        sdk->gameStates->Add(aHandle, RED4ext::EGameStateType::Running, &state);
    }
    {
        RED4ext::GameState state{nullptr, &HookShutdownStateTick, nullptr};
        sdk->gameStates->Add(aHandle, RED4ext::EGameStateType::Shutdown, &state);
    }
}

bool GameMainThread::HookBaseInitStateTick(RED4ext::CGameApplication* apGameApplication)
{
    auto& gmt = Get();

    // drain generic tasks
    gmt.m_genericQueue.Drain();

    gmt.m_stateTickOverrides[0].OnTick(apGameApplication->currState, apGameApplication);

    return true;
}

bool GameMainThread::HookInitStateTick(RED4ext::CGameApplication* apGameApplication)
{
    auto& gmt = Get();

    // drain generic tasks
    gmt.m_genericQueue.Drain();

    gmt.m_stateTickOverrides[1].OnTick(apGameApplication->currState, apGameApplication);

    return true;
}

bool GameMainThread::HookRunningStateTick(RED4ext::CGameApplication* apGameApplication)
{
    auto& gmt = Get();

    // drain generic tasks
    gmt.m_genericQueue.Drain();

    gmt.m_stateTickOverrides[2].OnTick(apGameApplication->currState, apGameApplication);

    return false;
}

bool GameMainThread::HookShutdownStateTick(RED4ext::CGameApplication* apGameApplication)
{
    auto& gmt = Get();

    // drain generic tasks
    gmt.m_genericQueue.Drain();

    gmt.m_stateTickOverrides[3].OnTick(apGameApplication->currState, apGameApplication);

    return true;
}
