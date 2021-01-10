#pragma once

#include "ToolbarWidget.h"
#include "ModWidgets.h"
#include "Console.h"
#include "Keybinds.h"
#include "Settings.h"

using TClipToCenter = HWND(RED4ext::CGameEngine::UnkC0*);

enum ToolbarWidgetID
{
    MODS,
    CONSOLE,
    KEYBINDS,
    SETTINGS,
    COUNT
};

struct Toolbar
{
    static void Initialize();
    static void PostInitialize();
    static void Shutdown();
    static Toolbar& Get();

    static ModWidgets& GetModWidgets();
    static Console& GetConsole();
    static Keybinds& GetKeybinds();
    static Settings& GetSettings();

    ~Toolbar() = default;
    
    bool IsInitialized() const;
    
    void Toggle();
    bool IsEnabled() const;

    void Update();

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

    static VKBind VKBToolbar;

protected:
    
    void Hook();
    
    static BOOL ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis);

private:

    Toolbar();

    void SetActiveWidget(ToolbarWidgetID aNewActive);

    ModWidgets m_mods{ };
    Console m_console{ };
    Keybinds m_keybinds{ };
    Settings m_settings{ };
    std::array<ToolbarWidget*, ToolbarWidgetID::COUNT> m_widgets{ }; 

    TClipToCenter* m_realClipToCenter{ nullptr };

    ToolbarWidgetID m_activeWidgetID{ ToolbarWidgetID::MODS };
    
    bool m_enabled{ false };
    bool m_initialized{ false };
};

inline VKBind Toolbar::VKBToolbar = { "toolbar_toggle",  [](){ Get().Toggle(); } };