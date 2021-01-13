#pragma once

#include "widgets/Widget.h"
#include "widgets/ModWidgets.h"
#include "widgets/Console.h"
#include "widgets/Hotkeys.h"
#include "widgets/Settings.h"

using TClipToCenter = HWND(RED4ext::CGameEngine::UnkC0*);

struct Overlay
{
    static void Initialize();
    static void PostInitialize();
    static void Shutdown();
    static Overlay& Get();
    
    static ModWidgets& GetModWidgets();
    static Console& GetConsole();
    static Hotkeys& GetHotkeys();
    static Settings& GetSettings();

    ~Overlay() = default;
    
    bool IsInitialized() const;
    
    void Toggle();
    bool IsEnabled() const;

    void Update();

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

    static VKBind VKBOverlay;

protected:
    
    void Hook();
    
    static BOOL ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis);

private:

    Overlay();

    void SetActiveWidget(WidgetID aNewActive);
    
    ModWidgets m_mods{ };
    Console m_console{ };
    Hotkeys m_hotkeys{ };
    Settings m_settings{ };
    std::array<Widget*, WidgetID::COUNT> m_widgets{ }; 

    TClipToCenter* m_realClipToCenter{ nullptr };

    WidgetID m_activeWidgetID{ WidgetID::MODS };
    
    bool m_enabled{ false };
    bool m_initialized{ false };
};

inline VKBind Overlay::VKBOverlay = { "cet.overlay_key",  "Overlay Key", [](){ Get().Toggle(); } };