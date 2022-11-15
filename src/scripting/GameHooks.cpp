#include <stdafx.h>

#include "GameHooks.h"

#include "RED4ext/GameStates.hpp"

static std::unique_ptr<GameMainThread> s_pGameMainThread;

void GameMainThread::TaskQueue::AddTask(const std::function<bool()>& aFunction)
{
    std::lock_guard lock(m_mutex);
    m_tasks.emplace_back(aFunction);
}

void GameMainThread::TaskQueue::Drain()
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

GameMainThread& GameMainThread::Get()
{
    static GameMainThread s_gameMainThread;
    return s_gameMainThread;
}

void GameMainThread::AddBaseInitializationTask(const std::function<bool()>& aFunction)
{
    m_baseInitializationQueue.AddTask(aFunction);
}

void GameMainThread::AddInitializationTask(const std::function<bool()>& aFunction)
{
    m_initializationQueue.AddTask(aFunction);
}

void GameMainThread::AddRunningTask(const std::function<bool()>& aFunction)
{
    m_runningQueue.AddTask(aFunction);
}

void GameMainThread::AddShutdownTask(const std::function<bool()>& aFunction)
{
    m_shutdownQueue.AddTask(aFunction);
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

bool GameMainThread::HookMainThreadStateTick(RED4ext::IGameState* apThisState, RED4ext::CGameApplication* apGameApplication)
{
    auto& gmt = Get();

    gmt.m_genericQueue.Drain();

    switch (apThisState->GetType())
    {
    case RED4ext::EGameStateType::BaseInitialization:
        gmt.m_baseInitializationQueue.Drain();
        return gmt.m_ppMainThreadStateTickLocations[0].second(apThisState, apGameApplication);
    case RED4ext::EGameStateType::Initialization:
        gmt.m_initializationQueue.Drain();
        return gmt.m_ppMainThreadStateTickLocations[1].second(apThisState, apGameApplication);
    case RED4ext::EGameStateType::Running:
        gmt.m_runningQueue.Drain();
        return gmt.m_ppMainThreadStateTickLocations[2].second(apThisState, apGameApplication);
    case RED4ext::EGameStateType::Shutdown:
        gmt.m_shutdownQueue.Drain();
        return gmt.m_ppMainThreadStateTickLocations[3].second(apThisState, apGameApplication);
    }

    // this function should never get here!
    assert(false);
    return false;
}

void GameMainThread::Hook()
{
    if (!m_ppMainThreadStateTickLocations[0].first)
    {
        const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CBaseInitializationState_OnTick);
        m_ppMainThreadStateTickLocations[0].first = func.GetAddr();
    }
    if (m_ppMainThreadStateTickLocations[0].first)
    {
        if (MH_CreateHook(m_ppMainThreadStateTickLocations[0].first, reinterpret_cast<void*>(&GameMainThread::HookMainThreadStateTick), reinterpret_cast<void**>(&m_ppMainThreadStateTickLocations[0].second)) != MH_OK ||
            MH_EnableHook(m_ppMainThreadStateTickLocations[0].first) != MH_OK)
            Log::Error("Could not hook main thread function CBaseInitializationState::OnTick()!");
    }
    else
        Log::Error("Could not locate CBaseInitializationState::OnTick()!");

    if (!m_ppMainThreadStateTickLocations[1].first)
    {
        const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CInitializationState_OnTick);
        m_ppMainThreadStateTickLocations[1].first = func.GetAddr();
    }
    if (m_ppMainThreadStateTickLocations[1].first)
    {
        if (MH_CreateHook(m_ppMainThreadStateTickLocations[1].first, reinterpret_cast<void*>(&GameMainThread::HookMainThreadStateTick), reinterpret_cast<void**>(&m_ppMainThreadStateTickLocations[1].second)) != MH_OK ||
            MH_EnableHook(m_ppMainThreadStateTickLocations[1].first) != MH_OK)
            Log::Error("Could not hook main thread function CInitializationState::OnTick()!");
    }
    else
        Log::Error("Could not locate CInitializationState::OnTick()!");

    if (!m_ppMainThreadStateTickLocations[2].first)
    {
        const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CRunningState_OnTick);
        m_ppMainThreadStateTickLocations[2].first = func.GetAddr();
    }
    if (m_ppMainThreadStateTickLocations[2].first)
    {
        if (MH_CreateHook(m_ppMainThreadStateTickLocations[2].first, reinterpret_cast<void*>(&GameMainThread::HookMainThreadStateTick), reinterpret_cast<void**>(&m_ppMainThreadStateTickLocations[2].second)) != MH_OK ||
            MH_EnableHook(m_ppMainThreadStateTickLocations[2].first) != MH_OK)
            Log::Error("Could not hook main thread function CRunningState::OnTick()!");
    }
    else
        Log::Error("Could not locate CRunningState::OnTick()!");

    if (!m_ppMainThreadStateTickLocations[3].first)
    {
        const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CShutdownState_OnTick);
        m_ppMainThreadStateTickLocations[3].first = func.GetAddr();
    }
    if (m_ppMainThreadStateTickLocations[3].first)
    {
        if (MH_CreateHook(m_ppMainThreadStateTickLocations[3].first, reinterpret_cast<void*>(&GameMainThread::HookMainThreadStateTick), reinterpret_cast<void**>(&m_ppMainThreadStateTickLocations[3].second)) != MH_OK ||
            MH_EnableHook(m_ppMainThreadStateTickLocations[3].first) != MH_OK)
            Log::Error("Could not hook main thread function CShutdownState::OnTick()!");
        else
            Log::Info("Main thread function hook complete!");
    }
    else
        Log::Error("Could not locate CShutdownState::OnTick()!");
}

void GameMainThread::Unhook() const
{
    for (auto location : m_ppMainThreadStateTickLocations | std::views::keys)
        MH_DisableHook(location);
}
