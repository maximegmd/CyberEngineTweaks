#pragma once

#include "Widget.h"

struct LuaVM;

struct Console : Widget
{
    Console(LuaVM& aVm);
    ~Console() override = default;

    WidgetResult OnEnable() override;
    WidgetResult OnDisable() override;
    void Update() override;

    void Log(const std::string& acpText);

private:
    static int HandleConsoleHistory(ImGuiInputTextCallbackData* apData);

    std::recursive_mutex m_outputLock{ };
    TiltedPhoques::Vector<std::pair<char, std::string>> m_outputLines{ };
    TiltedPhoques::Vector<std::string> m_consoleHistory{ };
    int64_t m_consoleHistoryIndex{ 0 };
    bool m_newConsoleHistory{ true };
    bool m_outputShouldScroll{ true };
    bool m_outputScroll{ false };
    bool m_focusConsoleInput{ false };
    LuaVM& m_vm;

    char m_Command[0x10000]{ 0 };
};