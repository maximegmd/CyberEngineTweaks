#pragma once

#include "Console.h"

using TClipToCenter = HWND(RED4ext::CGameEngine::UnkC0* apThis);

struct Toolbar
{
    static void Initialize();
    static void Shutdown();
    static Toolbar& Get();
    static Console& GetConsole();

    ~Toolbar();

    void Toggle();
    bool IsEnabled() const;

    void Update();

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

protected:
    
    void Hook();
    
    static BOOL ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis);

private:

    Toolbar();

    Console m_console{ };

    TClipToCenter* m_realClipToCenter{ nullptr };
    
    bool m_enabled{ false };
};
