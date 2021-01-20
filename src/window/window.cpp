#include <stdafx.h>

#include "CET.h"
#include "window.h"

#include <d3d12/D3D12.h>
#include <overlay/Overlay.h>

using namespace std::chrono_literals;

static Window* s_pWindow = nullptr;

static BOOL CALLBACK EnumWindowsProcCP77(HWND ahWnd, LPARAM alParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(ahWnd, &lpdwProcessId);
    if (lpdwProcessId == GetCurrentProcessId())
    {
        TCHAR name[512] = { 0 };
        GetWindowText(ahWnd, name, 511);
        if (_tcscmp(_T("Cyberpunk 2077 (C) 2020 by CD Projekt RED"), name) == 0)
        {
            *reinterpret_cast<HWND*>(alParam) = ahWnd;
            return FALSE;
        }
    }
    return TRUE;
}

LRESULT APIENTRY Window::WndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam)
{
    if (s_pWindow)
    {
        if (auMsg == WM_WINDOWPOSCHANGED)
        {
            auto* wp = reinterpret_cast<WINDOWPOS*>(alParam);
            s_pWindow->m_wndPos = {wp->x, wp->y};
            s_pWindow->m_wndSize = {wp->cx, wp->cy};

            RECT cr;
            GetClientRect(ahWnd, &cr);
            s_pWindow->m_clientPos = {cr.left, cr.top};
            s_pWindow->m_clientSize = {cr.right - cr.left, cr.bottom - cr.top};
        }

        {
            const auto res = s_pWindow->m_pBindings->OnWndProc(ahWnd, auMsg, awParam, alParam);
            if (res)
                return 0; // VKBindings wants this input ignored!
        }

        {
            const auto res = s_pWindow->m_pOverlay->OnWndProc(ahWnd, auMsg, awParam, alParam);
            if (res)
                return 0; // Toolbar wants this input ignored!
        }

        {
            const auto res = s_pWindow->m_pD3D12->OnWndProc(ahWnd, auMsg, awParam, alParam);
            if (res)
                return 0; // D3D12 wants this input ignored!
        }
    }
    
    return CallWindowProc(s_pWindow->m_wndProc, ahWnd, auMsg, awParam, alParam);
}

Window::Window(Overlay* apOverlay, VKBindings* apBindings, D3D12* apD3D12)
    : m_pOverlay(apOverlay)
    , m_pBindings(apBindings)
    , m_pD3D12(apD3D12)
{
    s_pWindow = this;

    std::thread t([this]()
    {
        while (m_hWnd == nullptr)
        {
            if (EnumWindows(EnumWindowsProcCP77, reinterpret_cast<LPARAM>(&m_hWnd)))
                std::this_thread::sleep_for(50ms);
            else
            {
                m_wndProc = reinterpret_cast<WNDPROC>(
                    SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
                spdlog::info("Window::Initialize() - window hook complete.");
            }
        }

        m_initialized = true;
    });

    t.detach();
}

Window::~Window() 
{
    s_pWindow = nullptr;

    if (m_hWnd != nullptr)
        SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_wndProc));
}
