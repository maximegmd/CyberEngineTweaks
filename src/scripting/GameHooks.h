#pragma once

struct GameMainThread
{
    static void Initialize();
    static void Shutdown();

    static GameMainThread& Get();

    void AddTask(const std::function<void()>& aFunction);
    void AddRepeatedTask(std::function<void()> aFunction);

    ~GameMainThread();

private:
    GameMainThread();

    void Hook();
    void Unhook() const;

    using TMainThreadLoop = bool(void*, void*);

    static bool HookMainThread(void* a1, void* a2);

    uint8_t* m_pMainThreadLocation = nullptr;
    TMainThreadLoop* m_pMainThreadOriginal = nullptr;

    TiltedPhoques::Vector<std::function<void()>> m_repeatedTasks;
    TiltedPhoques::TaskQueue m_taskQueue;
};