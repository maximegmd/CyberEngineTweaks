#pragma once

#include "widgets/Console.h"
#include "widgets/Bindings.h"
#include "widgets/Settings.h"
#include "widgets/TweakDBEditor.h"
#include "widgets/GameLog.h"
#include "widgets/ImGuiDebug.h"

using TClipToCenter = HWND(RED4ext::CGameEngine::UnkC0*);

struct Overlay
{
    Overlay(VKBindings& aBindings, Options& aOptions, PersistentState& aPersistentState, LuaVM& aVm);
    ~Overlay() = default;

    void PostInitialize();

    [[nodiscard]] bool IsInitialized() const noexcept;

    [[nodiscard]] Console& GetConsole();
    [[nodiscard]] Bindings& GetBindings();
    [[nodiscard]] Settings& GetSettings();

    void Toggle();
    [[nodiscard]] bool IsEnabled() const noexcept;

    void Update();

protected:
    void Hook();

    static BOOL ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis);

private:
    void DrawToolbar();

    Console m_console;
    Bindings m_bindings;
    Settings m_settings;
    TweakDBEditor m_tweakDBEditor;
    GameLog m_gameLog;
    ImGuiDebug m_imguiDebug;

    TClipToCenter* m_realClipToCenter{ nullptr };

    std::atomic_bool m_enabled{ false };
    std::atomic_bool m_toggled{ false };
    std::atomic_bool m_initialized{ false };

    Options& m_options;
    PersistentState& m_persistentState;
    LuaVM& m_vm;
};
