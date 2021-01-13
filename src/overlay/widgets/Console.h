#pragma once

#include "Widget.h"

struct Console : public Widget
{
    Console() = default;
    ~Console() override = default;
    
    void OnEnable() override;
    void OnDisable() override;
    void Update() override;
    
    void Log(std::string_view acpText);
    void GameLog(std::string_view acpText);

private:
    std::recursive_mutex m_outputLock{ };
    std::vector<std::string> m_outputLines{ };
    bool m_outputShouldScroll{ true };
    bool m_outputScroll{ false };
    bool m_inputClear{ true };
    bool m_disabledGameLog{ true };
    bool m_focusConsoleInput{ false };

    char m_Command[0x10000]{ 0 };
    int m_CommandLines{ 1 };
    float m_InputLineHeight{ -1.0f };
};