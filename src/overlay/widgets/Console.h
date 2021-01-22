#pragma once

#include "Widget.h"

struct LuaVM;

struct Console : Widget
{
    Console(LuaVM& aVm);
    ~Console() override = default;
    
    void OnEnable() override;
    void OnDisable() override;
    void Update() override;
    
    void Log(const std::string& acpText);
    bool GameLogEnabled() const;

private:
    std::recursive_mutex m_outputLock{ };
    std::vector<std::string> m_outputLines{ };
    bool m_outputShouldScroll{ true };
    bool m_outputScroll{ false };
    bool m_inputClear{ true };
    bool m_disabledGameLog{ true };
    bool m_focusConsoleInput{ false };
    LuaVM& m_vm;

    char m_Command[0x10000]{ 0 };
};