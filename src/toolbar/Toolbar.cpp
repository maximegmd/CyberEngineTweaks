#include <stdafx.h>

#include "Toolbar.h"

#include <Pattern.h>
#include <Options.h>

#include <d3d12/D3D12.h>
#include <scripting/LuaVM.h>

static std::unique_ptr<Toolbar> s_pToolbar;

void Toolbar::Initialize()
{
    if (!s_pToolbar)
    {
        s_pToolbar.reset(new (std::nothrow) Toolbar);
        s_pToolbar->Hook();
    }
}

void Toolbar::Shutdown()
{
    s_pToolbar = nullptr;
}

Toolbar& Toolbar::Get()
{
    assert(s_pToolbar);
    return *s_pToolbar;
}

Console& Toolbar::GetConsole()
{
    assert(s_pToolbar);
    return s_pToolbar->m_console;
}

void Toolbar::Update()
{
    if (!IsEnabled())
        return;

    m_console.Update();
}

void Toolbar::Toggle()
{
    m_enabled = !m_enabled;

    D3D12::Get().SetTrapInputInImGui(m_enabled);

    auto& luaVM = LuaVM::Get();
    if (m_enabled)
        luaVM.OnToolbarOpen();
    else
        luaVM.OnToolbarClose();
    
    ClipToCenter(RED4ext::CGameEngine::Get()->unkC0);

    m_console.Toggle();
}

bool Toolbar::IsEnabled() const
{
    return m_enabled;
}

LRESULT Toolbar::OnWndProc(HWND, UINT auMsg, WPARAM awParam, LPARAM)
{
    auto& options = Options::Get();
    if (auMsg == WM_KEYDOWN && awParam == options.ToolbarKey)
    {
        Toggle();
        return 1;
    }

    switch (auMsg)
    {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (awParam == options.ToolbarKey)
                return 1;
            break;
        case WM_CHAR:
            if (options.ToolbarChar && awParam == options.ToolbarChar)
                return 1;
            break;
    }

    return 0;
}

BOOL Toolbar::ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis)
{
    HWND wnd = (HWND)apThis->hWnd;
    HWND foreground = GetForegroundWindow();

    if(wnd == foreground && apThis->unk164 && !apThis->unk140 && !Get().IsEnabled())
    {
        RECT rect;
        GetClientRect(wnd, &rect);
        ClientToScreen(wnd, reinterpret_cast<POINT*>(&rect.left));
        ClientToScreen(wnd, reinterpret_cast<POINT*>(&rect.right));
        rect.left = (rect.left + rect.right) / 2;
        rect.right = rect.left;
        rect.bottom = (rect.bottom + rect.top) / 2;
        rect.top = rect.bottom;
        apThis->isClipped = true;
        ShowCursor(FALSE);
        return ClipCursor(&rect);
    }

    if(apThis->isClipped)
    {
        apThis->isClipped = false;
        return ClipCursor(nullptr);
    }

    return 1;
}

void Toolbar::Hook()
{
    uint8_t* pLocation = FindSignature({
        0x48, 0x89, 0x5C, 0x24, 0x08, 0x57, 0x48, 0x83, 0xEC, 0x30, 0x48, 0x8B,
        0x99, 0x68, 0x01, 0x00, 0x00, 0x48, 0x8B, 0xF9, 0xFF });

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &ClipToCenter, reinterpret_cast<void**>(&m_realClipToCenter)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            Logger::ErrorToMain("Could not hook mouse clip function!");
        else
            Logger::InfoToMain("Hook mouse clip function!");
    }
}

Toolbar::Toolbar() = default;

Toolbar::~Toolbar() = default;
