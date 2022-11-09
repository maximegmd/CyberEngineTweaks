#pragma once

#include "widgets/Console.h"
#include "widgets/Bindings.h"
#include "widgets/Settings.h"
#include "widgets/TweakDBEditor.h"
#include "widgets/GameLog.h"
#include "widgets/ImGuiDebug.h"

using TClipToCenter = HWND(RED4ext::CGameEngine::UnkC0*);

struct D3D12;
struct Options;
struct Overlay
{
    Overlay(D3D12& aD3D12, VKBindings& aBindings, Options& aOptions, LuaVM& aVm);
    ~Overlay();

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
    bool m_consoleEnabled = false;
    Bindings m_bindings;
    bool m_bindingsEnabled = false;
    Settings m_settings;
    bool m_settingsEnabled = false;
    TweakDBEditor m_tweakDBEditor;
    bool m_tweakDBEditorEnabled = false;
    GameLog m_gameLog;
    bool m_gameLogEnabled = false;
    ImGuiDebug m_imguiDebug;
    bool m_imguiDebugEnabled = false;

    TClipToCenter* m_realClipToCenter{ nullptr };

    std::atomic_bool m_enabled{ false };
    std::atomic_bool m_toggled{ false };
    bool m_initialized{ false };

    D3D12& m_d3d12;
    Options& m_options;
    LuaVM& m_vm;
    size_t m_connectInitialized;
};
