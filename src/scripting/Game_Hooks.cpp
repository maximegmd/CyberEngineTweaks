#include <stdafx.h>

#include "Game_Hooks.h"
#include <Pattern.h>


static std::unique_ptr<GameMainThread> s_pGameMainThread;

void GameMainThread::Initialize()
{
    if (!s_pGameMainThread)
    {
        s_pGameMainThread.reset(new (std::nothrow) GameMainThread);
        s_pGameMainThread->Hook();
    }
}

void GameMainThread::Shutdown()
{
    s_pGameMainThread->Unhook();
    s_pGameMainThread = nullptr;
}


GameMainThread& GameMainThread::Get()
{
    return *s_pGameMainThread;
}


void GameMainThread::AddTask(MainThreadTask* task)
{
    std::lock_guard<std::mutex> locker(m_queueLock);
    m_taskQueue.push(task);
}

bool GameMainThread::MainThread_Hooked(void* a1, void* a2)
{
    auto& gmt = GameMainThread::Get();
    while (!gmt.IsQueueEmpty())
    {
        gmt.m_queueLock.lock();
        MainThreadTask* cmd = gmt.m_taskQueue.front();
        gmt.m_taskQueue.pop();
        gmt.m_queueLock.unlock();

        cmd->Run();
        cmd->Dispose();
    }

    return gmt.m_pMainThreadOriginal(a1, a2);
}

void GameMainThread::Hook()
{
    if (!m_pMainThreadLocation)
    {
        // 40 55 57 41 57 48 81 EC ? ? ? ? 
        m_pMainThreadLocation = 
            FindSignature({0x40, 0x55, 0x57, 0x41, 0x57, 0x48, 0x81, 0xEC, 0xCC, 0xCC, 0xCC, 0xCC });
    }

    if (m_pMainThreadLocation)
    {
        if (MH_CreateHook(m_pMainThreadLocation, &GameMainThread::MainThread_Hooked, reinterpret_cast<void**>(&m_pMainThreadOriginal)) != MH_OK ||
            MH_EnableHook(m_pMainThreadLocation) != MH_OK)
            spdlog::error("Could not hook main thread function!");
        else
            spdlog::info("Main thread function hook complete!");
    }
}

void GameMainThread::Unhook()
{
    MH_DisableHook(m_pMainThreadLocation);
}

bool GameMainThread::IsQueueEmpty() const
{
    std::lock_guard<std::mutex> locker(m_queueLock);
    return m_taskQueue.empty();
}
