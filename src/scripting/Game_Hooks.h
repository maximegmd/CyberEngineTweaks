#pragma once

#include <queue>
#include <mutex>

struct MainThreadTask
{
    virtual void Run() = 0;
    virtual void Dispose() = 0;
};

struct GameMainThread
{
    static void Initialize();
    static void Shutdown();

    static GameMainThread& Get();

    void AddTask(MainThreadTask* task);

private:
    void Hook();
    void Unhook();
    bool IsQueueEmpty() const;

    using MainThreadLoop_t = bool(void*, void*);

    static bool MainThread_Hooked(void* a1, void* a2);

    uint8_t* m_pMainThreadLocation = nullptr;
    MainThreadLoop_t* m_pMainThreadOriginal = nullptr;

    mutable std::mutex m_queueLock;
    std::queue<MainThreadTask*> m_taskQueue;
};