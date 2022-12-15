#include <stdafx.h>

#include "scripting/LuaVM.h"

#include "EngineTweaks.h"

LuaVM::LuaVM(const Paths& aPaths, VKBindings& aBindings, D3D12& aD3D12)
    : m_scripting(aPaths, aBindings, aD3D12)
    , m_d3d12(aD3D12)
{
    Hook();

    aBindings.SetVM(this);
}

void LuaVM::Hook()
{
    GameMainThread::Get().AddRunningTask(
        [this]
        {
            const auto cNow = std::chrono::high_resolution_clock::now();
            const auto cDelta = cNow - m_lastframe;
            const auto cSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(cDelta);

            Update(cSeconds.count());

            m_lastframe = cNow;

            return false;
        });

    GameMainThread::Get().AddShutdownTask(
        [this]
        {
            m_scripting.UnloadAllMods();

            return true;
        });
}
