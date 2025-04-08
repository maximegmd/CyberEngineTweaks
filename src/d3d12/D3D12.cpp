#include <stdafx.h>

#include "D3D12.h"
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
    if (acEnabled)
    {
        // Trap input if it's the first request.
        if (m_trapInputStack == 0)
        {
            m_delayedTrapInputState = acEnabled;
            m_delayedTrapInput = true;
        }

        ++m_trapInputStack;
    }
    else
    {
        if (m_trapInputStack > 0)
        {
            --m_trapInputStack;

            // Disable when the last request is removed.
            if (m_trapInputStack == 0)
            {
                m_delayedTrapInputState = acEnabled;
                m_delayedTrapInput = true;
            }
        }
    }
}

LRESULT D3D12::OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam) const
{
    auto& d3d12 = CET::Get().GetD3D12();

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
