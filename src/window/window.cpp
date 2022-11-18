#include <stdafx.h>

#include "window.h"

#include <CET.h>

using namespace std::chrono_literals;

static Window* s_pWindow = nullptr;

LRESULT APIENTRY Window::WndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam)
{
    if (s_pWindow)
    {
        if (auMsg == WM_WINDOWPOSCHANGED)
        {
            const auto* wp = reinterpret_cast<WINDOWPOS*>(alParam);
            s_pWindow->m_wndPos = {wp->x, wp->y};
            s_pWindow->m_wndSize = {wp->cx, wp->cy};

            RECT cr;
            GetClientRect(ahWnd, &cr);
            s_pWindow->m_clientPos = {cr.left, cr.top};
            s_pWindow->m_clientSize = {cr.right - cr.left, cr.bottom - cr.top};
        }

        {
            if (s_pWindow->m_pBindings->OnWndProc(ahWnd, auMsg, awParam, alParam))
                return 0; // VKBindings wants this input ignored!
        }

        {
            if (s_pWindow->m_pD3D12->OnWndProc(ahWnd, auMsg, awParam, alParam))
                return 0; // D3D12 wants this input ignored!
        }

        return CallWindowProc(s_pWindow->m_wndProc, ahWnd, auMsg, awParam, alParam);
    }

    return 0;
}

Window::Window(VKBindings* apBindings, D3D12* apD3D12)
    : m_pBindings(apBindings)
    , m_pD3D12(apD3D12)
{
    s_pWindow = this;
}

void Window::Hook(HWND apWindowHandle)
{
    if (m_hWnd == apWindowHandle)
        return;

    m_initialized = false;

    if (m_hWnd && m_wndProc)
        SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_wndProc));

    m_hWnd = nullptr;
    m_wndProc = nullptr;
    m_wndPos = {0, 0};
    m_wndSize = {0, 0};
    m_clientPos = {0, 0};
    m_clientSize = {0, 0};

    Log::Info("Window::Hook() - window unhook complete!");

    if (!apWindowHandle)
        return;

    m_hWnd = apWindowHandle;

    m_wndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));

    RECT wr;
    GetWindowRect(m_hWnd, &wr);
    m_wndPos = {wr.left, wr.top};
    m_wndSize = {wr.right - wr.left, wr.bottom - wr.top};

    RECT cr;
    GetClientRect(m_hWnd, &cr);
    m_clientPos = {cr.left, cr.top};
    m_clientSize = {cr.right - cr.left, cr.bottom - cr.top};

    m_initialized = true;

    Log::Info("Window::Hook() - window hook complete!");
}

Window::~Window()
{
    s_pWindow = nullptr;

    assert(!m_initialized);
    if (m_initialized)
        Hook(nullptr);
}
