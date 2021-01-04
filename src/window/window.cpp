#include <stdafx.h>

#include "Window.h"

#include <Options.h>

#include <d3d12/D3D12.h>
#include <console/Console.h>

using namespace std::chrono_literals;

static BOOL CALLBACK EnumWindowsProcCP77(HWND hwnd, LPARAM lParam)
{
    DWORD lpdwProcessId;
    GetWindowThreadProcessId(hwnd, &lpdwProcessId);
    if (lpdwProcessId == GetCurrentProcessId())
    {
        char name[512] = { 0 };
        GetWindowTextA(hwnd, name, 511);
        if (strcmp("Cyberpunk 2077 (C) 2020 by CD Projekt RED", name) == 0)
        {
            *reinterpret_cast<HWND*>(lParam) = hwnd;
            return FALSE;
        }
    }
    return TRUE;
}

static std::unique_ptr<Window> s_pWindow;

void Window::Initialize()
{
    if (!s_pWindow)
    {
        s_pWindow.reset(new (std::nothrow) Window);
        std::thread t([]()
        {
            auto& window = Get();
            while (window.m_hWnd == nullptr) 
            {
                if (EnumWindows(EnumWindowsProcCP77, reinterpret_cast<LPARAM>(&window.m_hWnd)))
                    std::this_thread::sleep_for(50ms);
                else 
                {
                    window.m_wndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window.m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
                    spdlog::info("Window::Initialize() - window hook complete.");
                }
            }
        });
        t.detach();
    }
}

void Window::Shutdown()
{
    s_pWindow = nullptr;
}

Window& Window::Get()
{
    return *s_pWindow;
}

LRESULT APIENTRY Window::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    auto& window = Get();
    if (uMsg == WM_WINDOWPOSCHANGED)
    {
        auto* wp = reinterpret_cast<WINDOWPOS*>(lParam);
        window.m_wndPos = { wp->x, wp->y };
        window.m_wndSize = { wp->cx, wp->cy };

        RECT cr;
        GetClientRect(hWnd, &cr);
        window.m_clientPos = { cr.left, cr.top };
        window.m_clientSize = { cr.right - cr.left, cr.bottom - cr.top };
    }

    if (Options::Get().Console)
    {
        auto res = Console::Get().OnWndProc(hWnd, uMsg, wParam, lParam);
        if (res)
            return 0; // Console wants this input ignored!
    }
    
    {
        auto res = D3D12::Get().OnWndProc(hWnd, uMsg, wParam, lParam);
        if (res)
            return 0; // D3D12 wants this input ignored!
    }
    
    return CallWindowProc(window.m_wndProc, hWnd, uMsg, wParam, lParam);
}

Window::Window() = default;

Window::~Window() 
{
    if (m_hWnd != nullptr)
        SetWindowLongPtr(m_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_wndProc));
}
