#include <stdafx.h>

#include "GameHooks.h"

#include "RED4ext/GameStates.hpp"

static std::unique_ptr<GameMainThread> s_pGameMainThread;

void GameMainThread::Initialize()
{
    if (!s_pGameMainThread)
    {
        s_pGameMainThread.reset(new (std::nothrow) GameMainThread);
    }
}

void GameMainThread::Shutdown()
{
    s_pGameMainThread = nullptr;
}

GameMainThread& GameMainThread::Get()
{
    Initialize();
    return *s_pGameMainThread;
}

void GameMainThread::AddBaseInitializationTask(const std::function<bool()>& aFunction)
{
    m_baseInitializationTasks.emplace_back(aFunction);
}

void GameMainThread::AddInitializationTask(const std::function<bool()>& aFunction)
{
    m_initializationTasks.emplace_back(aFunction);
}

void GameMainThread::AddRunningTask(const std::function<bool()>& aFunction)
{
    m_runningTasks.emplace_back(aFunction);
}

void GameMainThread::AddShutdownTask(const std::function<bool()>& aFunction)
{
    m_shutdownTasks.emplace_back(aFunction);
}

void GameMainThread::AddGenericTask(const std::function<bool()>& aFunction)
{
    m_genericTasks.emplace_back(aFunction);
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

    for (auto taskIt = gmt.m_genericTasks.begin(); taskIt != gmt.m_genericTasks.end(); ++taskIt)
    {
        if ((*taskIt)())
            taskIt = gmt.m_genericTasks.erase(taskIt);
    }

    switch (apThisState->GetType())
    {
    case RED4ext::EGameStateType::BaseInitialization:
        for (auto taskIt = gmt.m_baseInitializationTasks.begin(); taskIt != gmt.m_baseInitializationTasks.end();)
        {
            if ((*taskIt)())
                taskIt = gmt.m_baseInitializationTasks.erase(taskIt);
            else
                ++taskIt;
        }
        return gmt.m_ppMainThreadStateTickLocations[0].second(apThisState, apGameApplication);
    case RED4ext::EGameStateType::Initialization:
        for (auto taskIt = gmt.m_initializationTasks.begin(); taskIt != gmt.m_initializationTasks.end();)
        {
            if ((*taskIt)())
                taskIt = gmt.m_initializationTasks.erase(taskIt);
            else
                ++taskIt;
        }
        return gmt.m_ppMainThreadStateTickLocations[1].second(apThisState, apGameApplication);
    case RED4ext::EGameStateType::Running:
        for (auto taskIt = gmt.m_runningTasks.begin(); taskIt != gmt.m_runningTasks.end();)
        {
            if ((*taskIt)())
                taskIt = gmt.m_runningTasks.erase(taskIt);
            else
                ++taskIt;
        }
        return gmt.m_ppMainThreadStateTickLocations[2].second(apThisState, apGameApplication);
    case RED4ext::EGameStateType::Shutdown:
        for (auto taskIt = gmt.m_shutdownTasks.begin(); taskIt != gmt.m_shutdownTasks.end();)
        {
            if ((*taskIt)())
                taskIt = gmt.m_shutdownTasks.erase(taskIt);
            else
                ++taskIt;
        }
        return gmt.m_ppMainThreadStateTickLocations[3].second(apThisState, apGameApplication);
    }

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
