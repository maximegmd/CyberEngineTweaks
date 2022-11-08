#include <stdafx.h>

#include "GameHooks.h"

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
    return *s_pGameMainThread;
}

void GameMainThread::AddTask(std::function<void()> aFunction)
{
    m_taskQueue.Add(std::move(aFunction));
}

GameMainThread::GameMainThread()
{
    Hook();
}

GameMainThread::~GameMainThread()
{
    Unhook();
}

bool GameMainThread::HookMainThread(void* a1, void* a2)
{
    auto& gmt = Get();

    gmt.m_taskQueue.Drain();

    return gmt.m_pMainThreadOriginal(a1, a2);
}

void GameMainThread::Hook()
{
    if (!m_pMainThreadLocation)
    {
        const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CGame_Main);
        m_pMainThreadLocation = func.GetAddr();
    }

    if (m_pMainThreadLocation)
    {
        if (MH_CreateHook(m_pMainThreadLocation, reinterpret_cast<void*>(&GameMainThread::HookMainThread), reinterpret_cast<void**>(&m_pMainThreadOriginal)) != MH_OK ||
            MH_EnableHook(m_pMainThreadLocation) != MH_OK)
            Log::Error("Could not hook main thread function!");
        else
            Log::Info("Main thread function hook complete!");
    }
}

void GameMainThread::Unhook() const
{
    MH_DisableHook(m_pMainThreadLocation);
}
