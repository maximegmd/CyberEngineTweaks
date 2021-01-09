#pragma once

struct Console
{
    Console() = default;
    ~Console() = default;

    void Toggle();
    bool IsEnabled() const;
    
    void Log(std::string_view acpText);
    void GameLog(std::string_view acpText);
    
    void Update();

private:
    std::recursive_mutex m_outputLock{ };
    std::vector<std::string> m_outputLines{ };
    bool m_outputShouldScroll{ true };
    bool m_outputScroll{ false };
    bool m_inputClear{ true };
    bool m_disabledGameLog{ true };
    
    bool m_enabled{ false };
    bool m_focusConsoleInput{ false };
};
