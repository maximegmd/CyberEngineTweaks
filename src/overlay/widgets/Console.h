#pragma once

#include "Widget.h"
#include "LogWindow.h"

struct D3D12;
struct LuaVM;
struct Console : Widget
{
    Console(D3D12& aD3D12, LuaVM& aVm);
    ~Console() override = default;

    WidgetResult OnDisable() override;

protected:
    void OnUpdate() override;

private:
    static int HandleConsoleHistory(ImGuiInputTextCallbackData* apData);
    static int HandleConsoleResize(ImGuiInputTextCallbackData* apData);
    static int HandleConsole(ImGuiInputTextCallbackData* apData);

    LuaVM& m_vm;
    LogWindow m_logWindow;

    TiltedPhoques::Vector<std::string> m_history;
    size_t m_historyIndex{ 0 };
    bool m_newHistory{ true };

    std::string m_command;
    int m_commandLength{ 0 };
};
