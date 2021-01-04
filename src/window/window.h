#pragma once

struct Window
{
    static void Initialize();
    static void Shutdown();
    static Window& Get();

    ~Window();

    HWND GetWindow() const { return m_hWnd; }

    POINT GetWndPos() const { return m_wndPos; }
    SIZE GetWndSize() const { return m_wndSize; }

    POINT GetClientPos() const { return m_clientPos; }
    SIZE GetClientSize() const { return m_clientSize; }

    bool IsInitialized() const { return m_initialized; }

protected:

    static LRESULT APIENTRY WndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

private:

    Window();
    
    bool m_initialized{ false };

    HWND m_hWnd{ nullptr };
    WNDPROC	m_wndProc{ nullptr };

    POINT m_wndPos{ };
    SIZE m_wndSize{ };
    POINT m_clientPos{ };
    SIZE m_clientSize{ };
};
