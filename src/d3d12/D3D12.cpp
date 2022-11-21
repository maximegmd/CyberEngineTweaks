#include <stdafx.h>

#include "D3D12.h"

#include <CET.h>
#include <imgui/imgui_impl_dx12.h>
#include <imgui/imgui_impl_win32.h>
#include <scripting/GameHooks.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void D3D12::SetTrapInputInImGui(const bool acEnabled)
{
    int showCursorState;
    if (acEnabled)
        do { showCursorState = ShowCursor(TRUE); } while (showCursorState < 0);
    else
        do { showCursorState = ShowCursor(FALSE); } while (showCursorState >= 0);

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

    {
        std::lock_guard stateGameLock(d3d12.m_stateGameMutex);

        if (const auto res = ImGui_ImplWin32_WndProcHandler(ahWnd, auMsg, awParam, alParam))
            return res;

        if (d3d12.m_delayedTrapInput)
        {
            d3d12.SetTrapInputInImGui(m_delayedTrapInputState);
            d3d12.m_delayedTrapInput = false;
        }
    }

    if (d3d12.m_trapInputInImGui) // TODO: look into io.WantCaptureMouse and io.WantCaptureKeyboard
    {
        // ignore mouse & keyboard events
        if ((auMsg >= WM_MOUSEFIRST && auMsg <= WM_MOUSELAST) ||
            (auMsg >= WM_KEYFIRST && auMsg <= WM_KEYLAST))
            return 1;

        // ignore input messages
        if (auMsg == WM_INPUT)
            return 1;
    }

    return 0;
}

D3D12::D3D12(Window& aWindow, Paths& aPaths, Options& aOptions)
    : m_paths(aPaths)
    , m_window(aWindow)
    , m_options(aOptions)
{
    m_fontSettings = m_options.Font;

    Hook();

    // add repeated task called in all stages excluding shutdown which prepares ImGui frame for next present
    GameMainThread::Get().AddBaseInitializationTask([this]{ PrepareUpdate(); return false; });
    GameMainThread::Get().AddInitializationTask([this]{ PrepareUpdate(); return false; });
    GameMainThread::Get().AddRunningTask([this]{ PrepareUpdate(); return false; });

    // add shutdown task which frees acquired resources
    GameMainThread::Get().AddShutdownTask([this]{ Shutdown(); return true; });
}

D3D12::~D3D12()
{
    assert(!m_initialized && m_shutdown);
    if (m_initialized)
        Shutdown();
}
