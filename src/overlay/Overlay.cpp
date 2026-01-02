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

#define EYE_ON_ICON "\xF3\xB0\x9B\x90"
#define EYE_OFF_ICON "\xF3\xB0\x9B\x91"


void Overlay::DrawToolbar()
{
    auto& persistentState = m_persistentState.Overlay;

    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.2f, 0.2f, 0.15f, 0.4f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, 15);

    if (ImGui::BeginMainMenuBar())
    {

        ImGui::Text(std::format("CyberEngineTweaks v{}", CET_GIT_TAG).c_str());

        ImGui::Separator();

        ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);

        if (ImGui::BeginMenu("Widgets"))
        {
            if (ImGui::MenuItem(std::format("{} Console", m_console.IsEnabled() ? EYE_ON_ICON : EYE_OFF_ICON).c_str()))
                m_console.Toggle();
            if (!m_toggled)
                persistentState.ConsoleToggled = m_console.IsEnabled();

            if (ImGui::MenuItem(std::format("{} Bindings", m_bindings.IsEnabled() ? EYE_ON_ICON : EYE_OFF_ICON).c_str()))
                m_bindings.Toggle();
            if (!m_toggled)
                persistentState.BindingsToggled = m_bindings.IsEnabled();

            if (ImGui::MenuItem(std::format("{} Settings", m_settings.IsEnabled() ? EYE_ON_ICON : EYE_OFF_ICON).c_str()))
                m_settings.Toggle();
            if (!m_toggled)
                persistentState.SettingsToggled = m_settings.IsEnabled();

            if (ImGui::MenuItem(std::format("{} TweakDB Editor", m_tweakDBEditor.IsEnabled() ? EYE_ON_ICON : EYE_OFF_ICON).c_str()))
                m_tweakDBEditor.Toggle();
            if (!m_toggled)
                persistentState.TweakDBEditorToggled = m_tweakDBEditor.IsEnabled();

            if (ImGui::MenuItem(std::format("{} Game Log", m_gameLog.IsEnabled() ? EYE_ON_ICON : EYE_OFF_ICON).c_str()))
                m_gameLog.Toggle();
            if (!m_toggled)
                persistentState.GameLogToggled = m_gameLog.IsEnabled();

            if (ImGui::MenuItem(std::format("{} ImGui Debug", m_imguiDebug.IsEnabled() ? EYE_ON_ICON : EYE_OFF_ICON).c_str()))
                m_imguiDebug.Toggle();
            if (!m_toggled)
                persistentState.ImGuiDebugToggled = m_imguiDebug.IsEnabled();

            ImGui::EndMenu();
        }

        ImGui::PopItemFlag();

        if (ImGui::BeginMenu("Reload Mods"))
        {
            ImGui::CloseCurrentPopup();
            m_vm.ReloadAllMods();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    ImGui::PopStyleVar();
}
