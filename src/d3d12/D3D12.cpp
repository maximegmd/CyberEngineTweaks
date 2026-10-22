#include <stdafx.h>

#include "D3D12.h"
#include "ShutdownCompletion.h"
#include "CET.h"

#include <imgui_impl/dx12.h>
#include <imgui_impl/win32.h>
#include <scripting/GameHooks.h>

void D3D12::SetTrapInputInImGui(const bool acEnabled)
{
    static const RED4ext::CName cReason = "ImGui";
    static RED4ext::UniversalRelocFunc<void (*)(RED4ext::CBaseEngine::UnkD0* apThis, RED4ext::CName aReason, bool aShow)>
        forceCursor(CyberEngineTweaks::AddressHashes::InputSystemWin32Base_ForceCursor);

    forceCursor(RED4ext::CGameEngine::Get()->unkD0, cReason, acEnabled);

    m_trapInputInImGui = acEnabled;
}

void D3D12::DelayedSetTrapInputInImGui(const bool acEnabled)
{
    m_delayedTrapInputState = acEnabled;
    m_delayedTrapInput = true;
}

LRESULT D3D12::OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam) const
{
    auto& d3d12 = CET::Get().GetD3D12();
    std::lock_guard lifecycleLock(d3d12.m_imguiLock);
    if (d3d12.m_shutdownStarted)
        return 0;

    if (d3d12.IsInitialized())
    {
        if (d3d12.m_delayedTrapInput)
        {
            d3d12.SetTrapInputInImGui(m_delayedTrapInputState);
            d3d12.m_delayedTrapInput = false;
        }

        if (d3d12.m_trapInputInImGui) // TODO: look into io.WantCaptureMouse and io.WantCaptureKeyboard
        {
            if (const LRESULT res = ImGui_ImplWin32_WndProcHandler(ahWnd, auMsg, awParam, alParam))
                return res;

            // ignore mouse & keyboard events
            if ((auMsg >= WM_MOUSEFIRST && auMsg <= WM_MOUSELAST) || (auMsg >= WM_KEYFIRST && auMsg <= WM_KEYLAST))
                return 1;

            // ignore input messages
            if (auMsg == WM_INPUT)
                return 1;
        }
    }

    return 0;
}

D3D12::D3D12(Window& aWindow, Paths& aPaths, Options& aOptions)
    : m_paths(aPaths)
    , m_window(aWindow)
    , m_options(aOptions)
{
    Hook();

    // add repeated task which prepares next ImGui frame for update
    GameMainThread::Get().AddGenericTask(
        [this]
        {
            PrepareUpdate();
            return false;
        });
}

D3D12::~D3D12()
{
    assert(!m_initialized);
}

void D3D12::BeginShutdown(ShutdownOrigin aOrigin)
{
    std::lock_guard lifecycleLock(m_imguiLock);
    if (m_shutdownStarted)
        return;
    m_shutdownStarted = true;
    const char* origin = aOrigin == ShutdownOrigin::AfterLuaUnload ? "after Lua unload" : "renderer fallback";
    Log::Info("D3D12 shutdown: origin={}; starting bounded GPU completion check", origin);
    spdlog::default_logger()->flush();

    if (m_pd3d12Device && m_pCommandQueue)
    {
        auto result = m_pd3d12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_shutdownFence));
        if (SUCCEEDED(result))
            result = m_pCommandQueue->Signal(m_shutdownFence.Get(), 1);
        if (FAILED(result))
        {
            Log::Error("D3D12 shutdown: GPU fence setup failed HRESULT={:X}; cleanup withheld", static_cast<uint32_t>(result));
            spdlog::default_logger()->flush();
            return;
        }

        const auto start = std::chrono::steady_clock::now();
        const auto completion = ShutdownCompletion::Wait(
            [this] { return m_shutdownFence->GetCompletedValue(); },
            [&start] { return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start).count()); },
            [] { Sleep(1); }, 1, 2000);
        if (completion != ShutdownCompletion::Result::Complete)
        {
            Log::Error("D3D12 shutdown: GPU completion failed reason={}; cleanup withheld",
                completion == ShutdownCompletion::Result::DeviceRemoved ? "device removed" : "2000ms timeout");
            spdlog::default_logger()->flush();
            return;
        }
    }
    else if (m_initialized)
    {
        Log::Error("D3D12 shutdown: initialized renderer lacks device/queue; cleanup withheld");
        spdlog::default_logger()->flush();
        return;
    }

    Log::Info("D3D12 shutdown: GPU completion check passed; origin={}", origin);
    spdlog::default_logger()->flush();
    ResetState(true);
    m_shutdownCleanupCompleted = true;
    Log::Info("D3D12 shutdown: cleanup completed; origin={}", origin);
    spdlog::default_logger()->flush();
}
