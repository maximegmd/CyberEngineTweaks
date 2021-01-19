#include <stdafx.h>

#include "Overlay.h"

#include "widgets/HelperWidgets.h"

#include <Pattern.h>
#include <Options.h>

#include <d3d12/D3D12.h>
#include <scripting/LuaVM.h>

static std::unique_ptr<Overlay> s_pOverlay;

void Overlay::Initialize()
{
    if (!s_pOverlay)
    {
        s_pOverlay.reset(new (std::nothrow) Overlay);
        s_pOverlay->Hook();
    }
}

void Overlay::PostInitialize()
{
    assert(s_pOverlay);
    if (!s_pOverlay->m_initialized)
    {
        if (Options::Get().IsFirstLaunch)
        {
            s_pOverlay->Toggle();
            s_pOverlay->m_activeWidgetID = WidgetID::SETTINGS;
        }
        s_pOverlay->m_widgets[s_pOverlay->m_activeWidgetID]->OnEnable();
        s_pOverlay->m_initialized = true;
    }
}

void Overlay::Shutdown()
{
    s_pOverlay = nullptr;
}

Overlay& Overlay::Get()
{
    assert(s_pOverlay);
    return *s_pOverlay;
}

ModWidgets& Overlay::GetModWidgets()
{
    return Get().m_mods;
}

Console& Overlay::GetConsole()
{
    return Get().m_console;
}

Hotkeys& Overlay::GetHotkeys()
{
    return Get().m_hotkeys;
}

Settings& Overlay::GetSettings()
{
    return Get().m_settings;
}

void Overlay::Toggle()
{
    m_enabled = !m_enabled;

    D3D12::Get().SetTrapInputInImGui(m_enabled);

    auto& luaVM = LuaVM::Get();
    if (m_enabled)
        luaVM.OnOverlayOpen();
    else
        luaVM.OnOverlayClose();
    
    ClipToCenter(RED4ext::CGameEngine::Get()->unkC0);
}

bool Overlay::IsEnabled() const
{
    return m_initialized && m_enabled;
}

void Overlay::Update()
{
    if (!IsEnabled())
        return;
    
    SIZE resolution = D3D12::Get().GetResolution();

    ImGui::SetNextWindowPos(ImVec2(resolution.cx * 0.2f, resolution.cy * 0.2f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(resolution.cx * 0.6f, resolution.cy * 0.6f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(420, 315), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Cyber Engine Tweaks"))
    {
        ImVec2 zeroVec = {0, 0};

        if (!Options::Get().IsFirstLaunch)
        {
            WidgetID selectedID = WidgetID::COUNT;
            selectedID = HelperWidgets::ToolbarWidget();
            if (selectedID < WidgetID::COUNT)
                SetActiveWidget(selectedID);

            if (m_activeWidgetID == WidgetID::MODS)
            {
                if (ImGui::BeginChild("Mod widgets", zeroVec, true))
                    m_mods.Update();
                ImGui::EndChild();
            }
            if (m_activeWidgetID == WidgetID::CONSOLE)
            {
                if (ImGui::BeginChild("Console", zeroVec, true))
                    m_console.Update();
                ImGui::EndChild();
            }
            if (m_activeWidgetID == WidgetID::HOTKEYS)
            {
                if (ImGui::BeginChild("Keybinds", zeroVec, true))
                    m_hotkeys.Update();
                ImGui::EndChild();
            }
        }
        if (m_activeWidgetID == WidgetID::SETTINGS)
        {
            if (ImGui::BeginChild("Settings", zeroVec, true))
                m_settings.Update();
            ImGui::EndChild();
        }
    }
    ImGui::End();
}

bool Overlay::IsInitialized() const
{
    return m_initialized;
}

LRESULT Overlay::OnWndProc(HWND, UINT auMsg, WPARAM awParam, LPARAM)
{
    // TODO - is this useful now?
    return 0;
}

BOOL Overlay::ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis)
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

void Overlay::Hook()
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

Overlay::Overlay()
{
    m_widgets[WidgetID::MODS] = &m_mods;
    m_widgets[WidgetID::CONSOLE] = &m_console;
    m_widgets[WidgetID::HOTKEYS] = &m_hotkeys;
    m_widgets[WidgetID::SETTINGS] = &m_settings;
}

void Overlay::SetActiveWidget(WidgetID aNewActive)
{
    if (m_activeWidgetID != aNewActive)
    {
        if (m_activeWidgetID < WidgetID::COUNT)
            m_widgets[m_activeWidgetID]->OnDisable();
        m_activeWidgetID = aNewActive;
        if (m_activeWidgetID < WidgetID::COUNT)
            m_widgets[m_activeWidgetID]->OnEnable();
    }
}
