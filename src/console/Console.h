#pragma once

using TClipToCenter = HWND(RED4ext::CGameEngine::UnkC0* apThis);

struct Image;
struct Console
{
    static void Initialize();
    static void Shutdown();
    static Console& Get();

    ~Console();

    void Toggle();
    bool IsEnabled() const;
    
    void Log(const std::string& acpText);
    void GameLog(const std::string& acpText) { if (!m_disabledGameLog) Log(acpText); }
    
    void Update();

    LRESULT OnWndProc(HWND ahWnd, UINT auMsg, WPARAM awParam, LPARAM alParam);

protected:
    
    void Hook();
    
    static BOOL ClipToCenter(RED4ext::CGameEngine::UnkC0* apThis);

private:

    Console();
    
    std::recursive_mutex m_outputLock;
    std::vector<std::string> m_outputLines;
    bool m_outputShouldScroll{ true };
    bool m_outputScroll{ false };
    bool m_inputClear{ true };
    bool m_disabledGameLog{ true };
    
    TClipToCenter* m_realClipToCenter{ nullptr };
    
    bool m_enabled{ false };
    bool m_focusConsoleInput{ false };
};
