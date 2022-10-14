#include <stdafx.h>

#include "D3D12.h"
#include "CET.h"

#include <kiero/kiero.h>
#include <imgui_impl/dx12.h>
#include <imgui_impl/win32.h>

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

    if (d3d12.IsInitialized())
    {
        const auto res = ImGui_ImplWin32_WndProcHandler(ahWnd, auMsg, awParam, alParam);
        if (res)
            return res;

        if (d3d12.m_delayedTrapInput)
        {
            d3d12.SetTrapInputInImGui(m_delayedTrapInputState);
            d3d12.m_delayedTrapInput = false;
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
    }

    return 0;
}

D3D12::D3D12(Window& aWindow, Paths& aPaths, Options& aOptions)
    : m_paths(aPaths)
    , m_window(aWindow)
    , m_options(aOptions)
{
    HookGame();

    std::thread t([this]
    {
        if (kiero::init() != kiero::Status::Success)
            Log::Error("Kiero failed!");
        else
        {
            std::string_view d3d12type = kiero::isDownLevelDevice() ? "D3D12on7" : "D3D12";
            Log::Info("Kiero initialized for {0}", d3d12type);

            Hook();
        }
    });

    t.detach();
}

D3D12::~D3D12()
{
    if (m_initialized)
    {
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
    }

    kiero::shutdown();
}
