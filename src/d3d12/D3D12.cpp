#include <stdafx.h>

#include "D3D12.h"
#include "EngineTweaks.h"

#include <imgui_impl/dx12.h>
#include <imgui_impl/win32.h>
#include <scripting/GameHooks.h>

void D3D12::SetTrapInputInImGui(const bool acEnabled)
{
    int showCursorState;
    if (acEnabled)
        do
        {
            showCursorState = ShowCursor(TRUE);
        } while (showCursorState < 0);
    else
        do
        {
            showCursorState = ShowCursor(FALSE);
        } while (showCursorState >= 0);

    m_trapInputInImGui = acEnabled;
}

void D3D12::DelayedSetTrapInputInImGui(const bool acEnabled)
{
    m_delayedTrapInputState = acEnabled;
    m_delayedTrapInput = true;
}

LRESULT D3D12::OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam) const
{
    auto& d3d12 = EngineTweaks::Get().GetD3D12();

    if (d3d12.IsInitialized())
    {
        if (const auto res = ImGui_ImplWin32_WndProcHandler(ahWnd, auMsg, awParam, alParam))
            return res;

        if (d3d12.m_delayedTrapInput)
        {
            d3d12.SetTrapInputInImGui(m_delayedTrapInputState);
            d3d12.m_delayedTrapInput = false;
        }

        if (d3d12.m_trapInputInImGui) // TODO: look into io.WantCaptureMouse and io.WantCaptureKeyboard
        {
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
    HookGame();

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
