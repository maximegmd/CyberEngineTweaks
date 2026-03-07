#include <stdafx.h>

#include "Overlay.h"

#include "config/CETVersion.h"

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
        }
        else
        {
            const auto cOverlayBindCode = CET::Get().GetBindings().GetBindCodeForModBind(Bindings::GetOverlayToggleModBind());
            ImGui::InsertNotification({ToastType::Info, NOTIFY_DEFAULT_DISMISS, "CET Overlay Bind: %s", VKBindings::GetBindString(cOverlayBindCode).c_str()});
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

        const auto& persistentState = m_persistentState.Overlay;

        auto drawPopup = false;
        WidgetResult disableResult;
        if (m_enabled)
        {
            if (m_toggled && !drawPopup && persistentState.ConsoleToggled)
            {
                disableResult = m_console.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && persistentState.BindingsToggled)
            {
                disableResult = m_bindings.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && persistentState.SettingsToggled)
            {
                disableResult = m_settings.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && persistentState.TweakDBEditorToggled)
            {
                disableResult = m_tweakDBEditor.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && persistentState.GameLogToggled)
            {
                disableResult = m_gameLog.OnDisable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::ENABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && persistentState.ImGuiDebugToggled)
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
            if (m_toggled && !drawPopup && persistentState.ConsoleToggled)
            {
                disableResult = m_console.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && persistentState.BindingsToggled)
            {
                disableResult = m_bindings.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && persistentState.SettingsToggled)
            {
                disableResult = m_settings.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && persistentState.TweakDBEditorToggled)
            {
                disableResult = m_tweakDBEditor.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && persistentState.GameLogToggled)
            {
                disableResult = m_gameLog.OnEnable();
                if (disableResult == WidgetResult::CANCEL)
                    m_toggled = false;
                if (disableResult == WidgetResult::DISABLED)
                    drawPopup = true;
            }

            if (m_toggled && !drawPopup && persistentState.ImGuiDebugToggled)
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
            m_toggled = false;
        }
    }

    // Notifications style setup
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f); // Disable round borders
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f); // Disable borders

    // Notifications color setup
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f)); // Background color

    // Main rendering function
    ImGui::RenderNotifications();

    //——————————————————————————————— WARNING ———————————————————————————————
    // Argument MUST match the amount of ImGui::PushStyleVar() calls
    ImGui::PopStyleVar(2);
    // Argument MUST match the amount of ImGui::PushStyleColor() calls
    ImGui::PopStyleColor(1);

    if (!m_enabled)
        return;

    DrawToolbar();

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

Overlay::Overlay(VKBindings& aBindings, Options& aOptions, PersistentState& aPersistentState, LuaVM& aVm)
    : m_console(aOptions, aPersistentState, aVm)
    , m_bindings(aBindings, aVm)
    , m_settings(aOptions, aVm)
    , m_tweakDBEditor(aVm)
    , m_options(aOptions)
    , m_persistentState(aPersistentState)
    , m_vm(aVm)
{
    GameMainThread::Get().AddBaseInitializationTask(
        [this]
        {
            PostInitialize();
            return true;
        });
}

Overlay::~Overlay()
{
}

bool MenuBarButton(const char* label)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
    ImVec2 pos = window->DC.CursorPos;

    ImVec2 text_pos(
        window->DC.CursorPos.x,
        pos.y + window->DC.CurrLineTextBaseOffset
    );



    bool pressed = ImGui::Selectable(
        std::format("##{}", label).c_str(),
        false,
        ImGuiSelectableFlags_NoHoldingActiveID |
        ImGuiSelectableFlags_NoSetKeyOwner |
        ImGuiSelectableFlags_SelectOnClick,
        ImVec2(label_size.x, label_size.y)
    );

    ImGui::RenderText(text_pos, label);

    window->DC.CursorPos.x += IM_TRUNC(style.ItemSpacing.x * (-1.0f + 0.5f));

    return pressed;
}

bool MenuBarToggle(const char* label, bool toggled)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 label_size = ImGui::CalcTextSize(label, nullptr, true);
    ImVec2 pos = window->DC.CursorPos;

    ImVec2 text_pos(
        window->DC.CursorPos.x,
        pos.y + window->DC.CurrLineTextBaseOffset
    );

    bool pressed = ImGui::Selectable(
        std::format("##{}", label).c_str(),
        toggled,                               // pass toggle state as 'selected'
        ImGuiSelectableFlags_NoHoldingActiveID |
        ImGuiSelectableFlags_NoSetKeyOwner |
        ImGuiSelectableFlags_SelectOnClick,
        ImVec2(label_size.x, label_size.y)
    );

    ImGui::RenderText(text_pos, label);

    window->DC.CursorPos.x += IM_TRUNC(style.ItemSpacing.x * (-1.0f + 0.5f));

    return pressed;
}

void Overlay::DrawToolbar()
{
    auto& persistentState = m_persistentState.Overlay;

    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.2f, 0.2f, 0.15f, 0.4f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, 15);
    if (ImGui::BeginMainMenuBar())
    {

        ImGui::Text(std::format("CyberEngineTweaks v{}", CET_GIT_TAG).c_str());

        ImGui::Separator();

        if (MenuBarToggle("Console", m_console.IsEnabled()))
            m_console.Toggle();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);

        if (MenuBarToggle("Bindings", m_bindings.IsEnabled()))
            m_bindings.Toggle();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);

        if (MenuBarToggle("Settings", m_settings.IsEnabled()))
            m_settings.Toggle();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);

        if (MenuBarToggle("TweakDB Editor", m_tweakDBEditor.IsEnabled()))
            m_tweakDBEditor.Toggle();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);

        if (MenuBarToggle("Game Log", m_gameLog.IsEnabled()))
            m_gameLog.Toggle();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);

        if (MenuBarToggle("ImGui Debug", m_imguiDebug.IsEnabled()))
            m_imguiDebug.Toggle();

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 5);

        if (!m_toggled)
        {
            persistentState.ConsoleToggled = m_console.IsEnabled();
            persistentState.BindingsToggled = m_bindings.IsEnabled();
            persistentState.SettingsToggled = m_settings.IsEnabled();
            persistentState.TweakDBEditorToggled = m_tweakDBEditor.IsEnabled();
            persistentState.GameLogToggled = m_gameLog.IsEnabled();
            persistentState.ImGuiDebugToggled = m_imguiDebug.IsEnabled();
        }

        if (MenuBarButton("Reload Mods"))
            m_vm.ReloadAllMods();

        ImGui::EndMainMenuBar();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    ImGui::PopStyleVar();
}
