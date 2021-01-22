#pragma once

struct Overlay;
struct D3D12;
struct VKBindings;

struct Window
{
    Window(Overlay* apOverlay, VKBindings* apBindings, D3D12* apD3D12);
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

    
    bool m_initialized{ false };

    HWND m_hWnd{ nullptr };
    WNDPROC	m_wndProc{ nullptr };

    POINT m_wndPos{ };
    SIZE m_wndSize{ };
    POINT m_clientPos{ };
    SIZE m_clientSize{ };

    Overlay* m_pOverlay;
    VKBindings* m_pBindings;
    D3D12* m_pD3D12;
};
