#include <stdafx.h>

#include "GameHooks.h"
#include "CET.h"

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
    auto& gmt = GameMainThread::Get();

    gmt.m_taskQueue.Drain();

    return gmt.m_pMainThreadOriginal(a1, a2);
}

void GameMainThread::Hook()
{
    if (!m_pMainThreadLocation)
    {
        auto& gameImage = CET::Get().GetOptions().GameImage;

        const mem::pattern cPattern("40 55 57 41 57 48 81 EC");
        const mem::default_scanner cScanner(cPattern);
        m_pMainThreadLocation = cScanner(gameImage.TextRegion).as<uint8_t*>();
    }

    if (m_pMainThreadLocation)
    {
        if (MH_CreateHook(m_pMainThreadLocation, &GameMainThread::HookMainThread, reinterpret_cast<void**>(&m_pMainThreadOriginal)) != MH_OK ||
            MH_EnableHook(m_pMainThreadLocation) != MH_OK)
            spdlog::error("Could not hook main thread function!");
        else
            spdlog::info("Main thread function hook complete!");
    }
}

void GameMainThread::Unhook() const
{
    MH_DisableHook(m_pMainThreadLocation);
}
