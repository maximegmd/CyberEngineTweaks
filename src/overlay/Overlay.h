#pragma once

#include "widgets/Console.h"
#include "widgets/Bindings.h"
#include "widgets/Settings.h"
#include "widgets/TweakDBEditor.h"
#include "widgets/GameLog.h"
#include "widgets/ImGuiDebug.h"

using TClipToCenter = HWND(RED4ext::CGameEngine::UnkD0*);

struct Overlay
{
    Overlay(VKBindings& aBindings, Options& aOptions, PersistentState& aPersistentState, LuaVM& aVm);
    ~Overlay();

    void PostInitialize();

    [[nodiscard]] bool IsInitialized() const noexcept;

    [[nodiscard]] Console& GetConsole();
    [[nodiscard]] Bindings& GetBindings();
    [[nodiscard]] Settings& GetSettings();

    void Toggle();
    [[nodiscard]] bool IsEnabled() const noexcept;

    void Update();

    void ShowNotification(const std::string& notification);
    void HideNotification();

protected:
    void Hook();

    static BOOL ClipToCenter(RED4ext::CGameEngine::UnkD0* apThis);

private:
    void DrawToolbar();
    void DrawNotification();

    Console m_console;
    Bindings m_bindings;
    Settings m_settings;
    TweakDBEditor m_tweakDBEditor;
    GameLog m_gameLog;
    ImGuiDebug m_imguiDebug;

    TClipToCenter* m_realClipToCenter{nullptr};

    std::atomic_bool m_enabled{false};
    std::atomic_bool m_toggled{false};
    bool m_initialized{false};
    std::atomic_bool m_drawNotification{false};
    std::chrono::time_point<std::chrono::steady_clock> m_startTime;
    bool m_bindHintShown{false};

    Options& m_options;
    PersistentState& m_persistentState;
    LuaVM& m_vm;

    std::string m_notificationString;
};
