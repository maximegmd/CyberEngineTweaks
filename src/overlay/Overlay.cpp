#include <stdafx.h>

#include "Overlay.h"

#include "CET.h"
#include "widgets/HelperWidgets.h"

#include <Options.h>

#include <d3d12/D3D12.h>
#include <scripting/LuaVM.h>

void Overlay::PostInitialize()
{
    if (!m_initialized)
    {
        if (m_options.IsFirstLaunch)
        {
            Toggle();
            m_activeWidgetID = WidgetID::SETTINGS;
        }
        m_widgets[static_cast<size_t>(m_activeWidgetID)]->OnEnable();
        m_initialized = true;
    }
}

Console& Overlay::GetConsole()
{
    return m_console;
}

Hotkeys& Overlay::GetHotkeys()
{
    return m_hotkeys;
}

Settings& Overlay::GetSettings()
{
    return m_settings;
}

void Overlay::Toggle()
{
    m_enabled = !m_enabled;

    auto& d3d12 = CET::Get().GetD3D12();
    d3d12.SetTrapInputInImGui(m_enabled);
    
    ClipToCenter(RED4ext::CGameEngine::Get()->unkC0);

    m_toggled = true;
}

bool Overlay::IsEnabled() const noexcept
{
    return m_initialized && m_enabled;
}

VKBind Overlay::GetBind() const noexcept
{
    return m_VKBOverlay;
}

void Overlay::Update()
{
    if (!m_initialized)
        return;

    // always check for this event in Update
    if (m_toggled)
    {
        if (m_enabled)
            m_vm.OnOverlayOpen();
        else
            m_vm.OnOverlayClose();
        m_toggled = false;
    }

    if (!m_enabled)
        return;

    auto& d3d12 = CET::Get().GetD3D12();

    const SIZE resolution = d3d12.GetResolution();

    ImGui::SetNextWindowPos(ImVec2(resolution.cx * 0.2f, resolution.cy * 0.2f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(resolution.cx * 0.6f, resolution.cy * 0.6f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(420, 315), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Cyber Engine Tweaks"))
    {
        const ImVec2 zeroVec = {0, 0};

        if (!m_options.IsFirstLaunch)
        {
            const WidgetID selectedID = HelperWidgets::ToolbarWidget();
            if (selectedID < WidgetID::COUNT)
                SetActiveWidget(selectedID);
            
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

bool Overlay::IsInitialized() const noexcept
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

    auto& overlay = CET::Get().GetOverlay();

    if (wnd == foreground && apThis->unk164 && !apThis->unk140 && !overlay.IsEnabled())
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
    const mem::pattern cClipToCenterPattern("48 89 5C 24 08 57 48 83 EC 30 48 8B 99 68 01 00 00 48 8B F9 FF");
    const mem::default_scanner scanner(cClipToCenterPattern);

    uint8_t* pLocation = scanner(m_options.GameImage.TextRegion).as<uint8_t*>();

    if (pLocation)
    {
        if (MH_CreateHook(pLocation, &ClipToCenter, reinterpret_cast<void**>(&m_realClipToCenter)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            spdlog::error("Could not hook mouse clip function!");
        else
            spdlog::info("Hook mouse clip function!");
    }
}

Overlay::Overlay(D3D12& aD3D12, VKBindings& aBindings, Options& aOptions, LuaVM& aVm)
    : m_console(aVm)
    , m_hotkeys(aBindings, *this, aVm)
    , m_settings(*this, aBindings, aOptions)
    , m_VKBOverlay{"cet.overlay_key", "Overlay Key", [this]() { Toggle(); }}
    , m_d3d12(aD3D12)
    , m_options(aOptions)
    , m_vm(aVm)
{
    m_widgets[static_cast<size_t>(WidgetID::CONSOLE)] = &m_console;
    m_widgets[static_cast<size_t>(WidgetID::HOTKEYS)] = &m_hotkeys;
    m_widgets[static_cast<size_t>(WidgetID::SETTINGS)] = &m_settings;

    Hook();

    aBindings.Load(*this);

    m_connectInitialized = aD3D12.OnInitialized.Connect([this]() { PostInitialize(); });
    m_connectUpdate = aD3D12.OnUpdate.Connect([this]() { Update(); });
}

Overlay::~Overlay()
{
    m_d3d12.OnInitialized.Disconnect(m_connectInitialized);
    m_d3d12.OnUpdate.Disconnect(m_connectUpdate);
}

void Overlay::SetActiveWidget(WidgetID aNewActive)
{
    if (m_activeWidgetID != aNewActive)
    {
        if (m_activeWidgetID < WidgetID::COUNT)
            m_widgets[static_cast<size_t>(m_activeWidgetID)]->OnDisable();
        m_activeWidgetID = aNewActive;
        if (m_activeWidgetID < WidgetID::COUNT)
            m_widgets[static_cast<size_t>(m_activeWidgetID)]->OnEnable();
    }
}
