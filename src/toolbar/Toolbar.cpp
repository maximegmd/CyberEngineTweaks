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

void Toolbar::PostInitialize()
{
    assert(s_pToolbar);
    if (!s_pToolbar->m_initialized)
    {
        if (Options::IsFirstLaunch)
        {
            s_pToolbar->Toggle();
            s_pToolbar->m_activeWidgetID = ToolbarWidgetID::SETTINGS;
        }
        s_pToolbar->m_widgets[s_pToolbar->m_activeWidgetID]->OnEnable();
        s_pToolbar->m_initialized = true;
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

ModWidgets& Toolbar::GetModWidgets()
{
    assert(s_pToolbar);
    return s_pToolbar->m_mods;
}

Console& Toolbar::GetConsole()
{
    assert(s_pToolbar);
    return s_pToolbar->m_console;
}

Keybinds& Toolbar::GetKeybinds()
{
    assert(s_pToolbar);
    return s_pToolbar->m_keybinds;
}

Settings& Toolbar::GetSettings()
{
    assert(s_pToolbar);
    return s_pToolbar->m_settings;
}

void Toolbar::Update()
{
    if (!IsEnabled())
        return;
    
    SIZE resolution = D3D12::Get().GetResolution();

    ImGui::SetNextWindowPos(ImVec2(resolution.cx * 0.2f, resolution.cy * 0.2f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(resolution.cx * 0.6f, resolution.cy * 0.6f), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Cyber Engine Tweaks", nullptr, ImGuiWindowFlags_NoScrollbar))
    {
        if (ImGui::BeginTabBar("CET_TABS", ImGuiTabBarFlags_None))
        {
            constexpr int tabFlags = ImGuiTabItemFlags_NoReorder | ImGuiTabItemFlags_NoCloseWithMiddleMouseButton;
            if (!Options::IsFirstLaunch)
            {
                if (ImGui::BeginTabItem("Mod widgets", nullptr, tabFlags))
                {
                    SetActiveWidget(ToolbarWidgetID::MODS);
                    m_mods.Update();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Console", nullptr, tabFlags))
                {
                    SetActiveWidget(ToolbarWidgetID::CONSOLE);
                    m_console.Update();
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Keybinds", nullptr, tabFlags))
                {
                    SetActiveWidget(ToolbarWidgetID::KEYBINDS);
                    m_keybinds.Update();
                    ImGui::EndTabItem();
                }
            }
            if (ImGui::BeginTabItem("Settings", nullptr, tabFlags | ImGuiTabItemFlags_Trailing))
            {
                SetActiveWidget(ToolbarWidgetID::SETTINGS);
                m_settings.Update();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
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
}

bool Toolbar::IsEnabled() const
{
    return m_initialized && m_enabled;
}

LRESULT Toolbar::OnWndProc(HWND, UINT auMsg, WPARAM awParam, LPARAM)
{
    if (!m_initialized)
        return 0; // we have not yet fully initialized!

    if (m_settings.IsBindingKey())
    {
        switch (auMsg)
        {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                m_settings.RecordKeyDown(awParam);
                return 1;
            case WM_KEYUP:
            case WM_SYSKEYUP:
                m_settings.RecordKeyUp(awParam);
                return 1;
        }
        return 0;
    }

    if (Options::IsFirstLaunch)
        return 0; // do not handle anything until user sets his settings for the first time

    switch (auMsg)
    {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            if (awParam == Options::ToolbarKey)
            {
                Toggle();
                return 1;
            }
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            if (awParam == Options::ToolbarKey)
                return 1;
            break;
        case WM_CHAR:
            if (awParam == Options::ToolbarChar)
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

Toolbar::Toolbar()
{
    m_widgets[ToolbarWidgetID::MODS] = &m_mods;
    m_widgets[ToolbarWidgetID::CONSOLE] = &m_console;
    m_widgets[ToolbarWidgetID::KEYBINDS] = &m_keybinds;
    m_widgets[ToolbarWidgetID::SETTINGS] = &m_settings;
}

void Toolbar::SetActiveWidget(ToolbarWidgetID aNewActive)
{
    if (m_activeWidgetID != aNewActive)
    {
        m_widgets[m_activeWidgetID]->OnDisable();
        m_activeWidgetID = aNewActive;
        m_widgets[m_activeWidgetID]->OnEnable();
    }
}
