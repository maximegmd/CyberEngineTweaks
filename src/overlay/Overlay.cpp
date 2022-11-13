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
            Toggle();

            auto& d3d12 = CET::Get().GetD3D12();
            d3d12.DelayedSetTrapInputInImGui(true);
            ClipToCenter(RED4ext::CGameEngine::Get()->unkC0);
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
    // return true also when overlay is closed but toggling on/off
    return m_initialized && (m_enabled || m_toggled);
}

void Overlay::Update()
{
    if (!m_initialized)
        return;

    if (m_toggled)
    {
        if (m_bindings.FirstTimeSetup())
            return;

        auto drawPopup = false;
        WidgetResult disableResult;
        if (m_enabled)
        {
            if (m_toggled && !drawPopup && m_consoleEnabled)
            {
                disableResult = m_console.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && m_bindingsEnabled)
            {
                disableResult = m_bindings.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && m_settingsEnabled)
            {
                disableResult = m_settings.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && m_tweakDBEditorEnabled)
            {
                disableResult = m_tweakDBEditor.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && m_gameLogEnabled)
            {
                disableResult = m_gameLog.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && m_imguiDebugEnabled)
            {
                disableResult = m_imguiDebug.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }
        }
        else
        {
            if (m_toggled && !drawPopup && m_consoleEnabled)
            {
                disableResult = m_console.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && m_bindingsEnabled)
            {
                disableResult = m_bindings.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && m_settingsEnabled)
            {
                disableResult = m_settings.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && m_tweakDBEditorEnabled)
            {
                disableResult = m_tweakDBEditor.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && m_gameLogEnabled)
            {
                disableResult = m_gameLog.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && m_imguiDebugEnabled)
            {
                disableResult = m_imguiDebug.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }
        }

        if (!drawPopup && m_toggled)
        {
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
    }

    if (!m_enabled)
        return;

    const auto [width, height] = CET::Get().GetD3D12().GetResolution();
    const auto heightLimit = 2 * ImGui::GetFrameHeight() + 2 * ImGui::GetStyle().WindowPadding.y;
    ImGui::SetNextWindowPos({width * 0.25f, height * 0.05f}, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSizeConstraints({width * 0.5f, heightLimit}, {FLT_MAX, heightLimit});
    if (ImGui::Begin("Cyber Engine Tweaks"))
        DrawToolbar();
    ImGui::End();

    m_console.Draw();
    m_bindings.Draw();
    m_settings.Draw();
    m_tweakDBEditor.Draw();
    m_gameLog.Draw();
    m_imguiDebug.Draw();
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
    Hook();

    m_connectInitialized = aD3D12.OnInitialized.Connect([this]{ PostInitialize(); });
}

Overlay::~Overlay()
{
    m_d3d12.OnInitialized.Disconnect(m_connectInitialized);
}

void Overlay::DrawToolbar()
{
    const auto itemWidth = GetAlignedItemWidth(7);

    ImGui::PushStyleColor(ImGuiCol_Button, m_consoleEnabled ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("Console", ImVec2(itemWidth, 0)))
        m_console.Toggle();
    if (!m_toggled)
        m_consoleEnabled = m_console.IsEnabled();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, m_bindingsEnabled ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("Bindings", ImVec2(itemWidth, 0)))
        m_bindings.Toggle();
    if (!m_toggled)
        m_bindingsEnabled = m_bindings.IsEnabled();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, m_settingsEnabled ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("Settings", ImVec2(itemWidth, 0)))
        m_settings.Toggle();
    if (!m_toggled)
        m_settingsEnabled = m_settings.IsEnabled();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, m_tweakDBEditorEnabled ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("TweakDB Editor", ImVec2(itemWidth, 0)))
        m_tweakDBEditor.Toggle();
    if (!m_toggled)
        m_tweakDBEditorEnabled = m_tweakDBEditor.IsEnabled();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, m_gameLogEnabled ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("Game Log", ImVec2(itemWidth, 0)))
        m_gameLog.Toggle();
    if (!m_toggled)
        m_gameLogEnabled = m_gameLog.IsEnabled();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, m_imguiDebugEnabled ? ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive) : ImGui::GetStyleColorVec4(ImGuiCol_Button));
    if (ImGui::Button("ImGui Debug", ImVec2(itemWidth, 0)))
        m_imguiDebug.Toggle();
    if (!m_toggled)
        m_imguiDebugEnabled = m_imguiDebug.IsEnabled();
    ImGui::PopStyleColor();

    ImGui::SameLine();

    if (ImGui::Button("Reload all mods", ImVec2(itemWidth, 0)))
        m_vm.ReloadAllMods();
}
