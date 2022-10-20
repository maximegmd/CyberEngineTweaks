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
            Toggle();

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
        if (m_bindings.FirstTimeSetup())
            return;

        if (m_enabled)
            m_vm.OnOverlayClose();
        else
            m_vm.OnOverlayOpen();
        m_enabled = !m_enabled;

        auto& d3d12 = CET::Get().GetD3D12();
        d3d12.DelayedSetTrapInputInImGui(m_enabled);
        ClipToCenter(RED4ext::CGameEngine::Get()->unkC0);
        m_toggled = false;
    }

    if (!m_enabled)
        return;

    const auto minWidth = CET::Get().GetD3D12().GetResolution().cx * 0.5f;
    const auto heightLimit = 2 * ImGui::GetFrameHeight() + 2 * ImGui::GetStyle().WindowPadding.y;
    ImGui::SetNextWindowSizeConstraints({minWidth, heightLimit}, {FLT_MAX, heightLimit});
    if (ImGui::Begin("Cyber Engine Tweaks"))
        DrawToolbar();
    ImGui::End();

    m_console.Draw();
    m_bindings.Draw();
    m_settings.Draw();
    m_tweakDBEditor.Draw();
    m_gameLog.Draw();

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
    : m_console(aD3D12, aVm)
    , m_bindings(aBindings, aVm)
    , m_settings(aOptions, aVm)
    , m_tweakDBEditor(aVm)
    , m_gameLog(aD3D12)
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

void Overlay::DrawToolbar()
{
    const auto itemWidth = GetAlignedItemWidth(static_cast<int64_t>(WidgetID::COUNT) + 1);

    ImGui::PushStyleColor(ImGuiCol_Button, m_console.IsEnabled() ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("Console", ImVec2(itemWidth, 0)))
        m_console.Toggle();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, m_bindings.IsEnabled() ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("Bindings", ImVec2(itemWidth, 0)))
        m_bindings.Toggle();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, m_settings.IsEnabled() ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("Settings", ImVec2(itemWidth, 0)))
        m_settings.Toggle();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, m_tweakDBEditor.IsEnabled() ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("TweakDB Editor", ImVec2(itemWidth, 0)))
        m_tweakDBEditor.Toggle();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, m_gameLog.IsEnabled() ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("Game Log", ImVec2(itemWidth, 0)))
        m_gameLog.Toggle();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    if (ImGui::Button("Reload all mods", ImVec2(itemWidth, 0)))
        m_vm.ReloadAllMods();
}
