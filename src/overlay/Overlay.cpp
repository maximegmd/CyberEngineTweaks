#include <stdafx.h>

#include "Overlay.h"

#include <CET.h>

#include <d3d12/D3D12.h>
#include <scripting/LuaVM.h>
#include <Utils.h>

void Overlay::PostInitialize()
{
    if (!m_initialized)
    {
        if (Bindings::IsFirstTimeSetup())
        {
            SetActiveWidget(WidgetID::BINDINGS);
            Toggle();
        }

        m_initialized = true;
    }
}

Console& Overlay::GetConsole()
{
    return m_console;
}

Bindings& Overlay::GetBindings()
{
    return m_bindings;
}

Settings& Overlay::GetSettings()
{
    return m_settings;
}

void Overlay::Toggle()
{
    if (!m_toggled)
        m_toggled = true;
}

bool Overlay::IsEnabled() const noexcept
{
    return m_initialized && m_enabled;
}

void Overlay::Update()
{
    if (!m_initialized)
        return;

    if (m_toggled)
    {
        if (Bindings::IsFirstTimeSetup())
        {
            ImGui::OpenPopup("CET First Time Setup");
            m_vm.BlockDraw(true);
        }

        if (m_enabled)
        {
            const auto ret = m_widgets[static_cast<size_t>(m_activeWidgetID)]->OnDisable();
            if (ret == WidgetResult::CANCEL)
            {
                m_toggled = false;
            }
            if (ret == WidgetResult::DISABLED)
            {
                m_vm.OnOverlayClose();
                m_enabled = false;
                m_toggled = false;
            }
        }
        else
        {
            const auto ret = m_widgets[static_cast<size_t>(m_activeWidgetID)]->OnEnable();
            if (ret == WidgetResult::CANCEL)
            {
                m_toggled = false;
            }
            if (ret == WidgetResult::ENABLED)
            {
                m_vm.OnOverlayOpen();
                m_enabled = true;
                m_toggled = false;
            }
        }

        if (!m_toggled)
        {
            auto& d3d12 = CET::Get().GetD3D12();
            d3d12.DelayedSetTrapInputInImGui(m_enabled);
            ClipToCenter(RED4ext::CGameEngine::Get()->unkC0);
        }
    }

    if (!m_enabled)
        return;

    if (m_bindings.FirstTimeSetup())
        return;

    const auto& d3d12 = CET::Get().GetD3D12();
    const auto [width, height] = d3d12.GetResolution();

    ImGui::SetNextWindowPos(ImVec2(width * 0.2f, height * 0.2f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(width * 0.6f, height * 0.6f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints(ImVec2(420, 315), ImVec2(FLT_MAX, FLT_MAX));
    if (ImGui::Begin("Cyber Engine Tweaks"))
    {
        SetActiveWidget(ToolbarWidget());

        if (m_activeWidgetID == WidgetID::CONSOLE)
        {
            if (ImGui::BeginChildFrame(ImGui::GetID("ConsoleWidget"), ImGui::GetContentRegionAvail()))
                m_console.Update();
            ImGui::EndChildFrame();
        }
        if (m_activeWidgetID == WidgetID::BINDINGS)
        {
            if (ImGui::BeginChildFrame(ImGui::GetID("BindingsWidget"), ImGui::GetContentRegionAvail()))
                m_bindings.Update();
            ImGui::EndChildFrame();
        }
        if (m_activeWidgetID == WidgetID::SETTINGS)
        {
            if (ImGui::BeginChildFrame(ImGui::GetID("SettingsWidget"), ImGui::GetContentRegionAvail()))
                m_settings.Update();
            ImGui::EndChildFrame();
        }
        if (m_activeWidgetID == WidgetID::TWEAKDB)
        {
            if (ImGui::BeginChildFrame(ImGui::GetID("TweakDBWidget"), ImGui::GetContentRegionAvail()))
                m_tweakDBEditor.Update();
            ImGui::EndChildFrame();
        }
        if (m_activeWidgetID == WidgetID::GAMELOG)
        {
            if (ImGui::BeginChildFrame(ImGui::GetID("GameLogWidget"), ImGui::GetContentRegionAvail()))
                m_gameLog.Update();
            ImGui::EndChildFrame();
        }
    }
    ImGui::End();

    if (m_options.DrawImGuiDiagnosticWindow)
        ImGui::ShowMetricsWindow(&m_options.DrawImGuiDiagnosticWindow);
}

bool Overlay::IsInitialized() const noexcept
{
    return m_initialized;
}

BOOL Overlay::ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis)
{
    const auto wnd = static_cast<HWND>(apThis->hWnd);
    const HWND foreground = GetForegroundWindow();

    if (wnd == foreground && apThis->unk164 && !apThis->unk154 && !CET::Get().GetOverlay().IsEnabled())
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
    const RED4ext::RelocPtr<uint8_t> func(CyberEngineTweaks::Addresses::CWinapi_ClipToCenter);

    if (auto* pLocation = func.GetAddr())
    {
        if (MH_CreateHook(pLocation, reinterpret_cast<void*>(&ClipToCenter), reinterpret_cast<void**>(&m_realClipToCenter)) != MH_OK || MH_EnableHook(pLocation) != MH_OK)
            Log::Error("Could not hook mouse clip function!");
        else
            Log::Info("Hook mouse clip function!");
    }
}

Overlay::Overlay(D3D12& aD3D12, VKBindings& aBindings, Options& aOptions, LuaVM& aVm)
    : m_console(aVm)
    , m_bindings(aBindings, aVm)
    , m_settings(aOptions, aVm)
    , m_tweakDBEditor(aVm)
    , m_d3d12(aD3D12)
    , m_options(aOptions)
    , m_vm(aVm)
{
    m_widgets[static_cast<size_t>(WidgetID::CONSOLE)] = &m_console;
    m_widgets[static_cast<size_t>(WidgetID::BINDINGS)] = &m_bindings;
    m_widgets[static_cast<size_t>(WidgetID::SETTINGS)] = &m_settings;
    m_widgets[static_cast<size_t>(WidgetID::TWEAKDB)] = &m_tweakDBEditor;
    m_widgets[static_cast<size_t>(WidgetID::GAMELOG)] = &m_gameLog;

    Hook();

    m_connectInitialized = aD3D12.OnInitialized.Connect([this]{ PostInitialize(); });
    m_connectUpdate = aD3D12.OnUpdate.Connect([this]{ Update(); });
}

Overlay::~Overlay()
{
    m_d3d12.OnInitialized.Disconnect(m_connectInitialized);
    m_d3d12.OnUpdate.Disconnect(m_connectUpdate);
}

void Overlay::SetActiveWidget(WidgetID aNewActive)
{
    if (aNewActive < WidgetID::COUNT)
        m_nextActiveWidgetID = aNewActive;

    if (m_activeWidgetID != m_nextActiveWidgetID)
    {
        assert(m_activeWidgetID < WidgetID::COUNT);

        if (m_widgets[static_cast<size_t>(m_activeWidgetID)]->OnDisable() == WidgetResult::DISABLED)
        {
            assert(m_nextActiveWidgetID < WidgetID::COUNT);
            if (m_widgets[static_cast<size_t>(m_nextActiveWidgetID)]->OnEnable() == WidgetResult::ENABLED)
                m_activeWidgetID = m_nextActiveWidgetID;
        }
    }
}

WidgetID Overlay::ToolbarWidget() const
{
    const auto itemWidth = GetAlignedItemWidth(static_cast<int64_t>(WidgetID::COUNT) + 1);

    auto activeID = WidgetID::COUNT;
    if (ImGui::Button("Console", ImVec2(itemWidth, 0)))
        activeID = WidgetID::CONSOLE;
    ImGui::SameLine();
    if (ImGui::Button("Bindings", ImVec2(itemWidth, 0)))
        activeID = WidgetID::BINDINGS;
    ImGui::SameLine();
    if (ImGui::Button("Settings", ImVec2(itemWidth, 0)))
        activeID = WidgetID::SETTINGS;
    ImGui::SameLine();
    if (ImGui::Button("TweakDB Editor", ImVec2(itemWidth, 0)))
        activeID = WidgetID::TWEAKDB;
    ImGui::SameLine();
    if (ImGui::Button("Game Log", ImVec2(itemWidth, 0)))
        activeID = WidgetID::GAMELOG;
    ImGui::SameLine();
    if (ImGui::Button("Reload all mods", ImVec2(itemWidth, 0)))
        m_vm.ReloadAllMods();
    ImGui::Separator();

    return activeID;
}
