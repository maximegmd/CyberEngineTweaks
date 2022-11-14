#pragma once

#include "LogWindow.h"

struct LuaVM;
struct Console : LogWindow
{
    Console(LuaVM& aVm);
    ~Console() override = default;

    WidgetResult OnDisable() override;

protected:
    void OnUpdate() override;

private:
    static int HandleConsoleHistory(ImGuiInputTextCallbackData* apData);
    static int HandleConsoleResize(ImGuiInputTextCallbackData* apData);
    static int HandleConsole(ImGuiInputTextCallbackData* apData);

    LuaVM& m_vm;

    TiltedPhoques::Vector<std::string> m_history;
    size_t m_historyIndex{ 0 };
    bool m_newHistory{ true };

    std::string m_command;
    int m_commandLength{ 0 };
};
