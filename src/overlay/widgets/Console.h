#pragma once

#include "Widget.h"

struct D3D12;
struct LuaVM;
struct Console : Widget
{
    Console(LuaVM& aVm, D3D12& aD3D12);
    ~Console() override = default;

    WidgetResult OnEnable() override;
    WidgetResult OnDisable() override;
    void Update() override;

    void Log(const std::string& acpText);

private:
    static int HandleConsoleHistory(ImGuiInputTextCallbackData* apData);
    static int HandleConsoleResize(ImGuiInputTextCallbackData* apData);
    static int HandleConsole(ImGuiInputTextCallbackData* apData);

    std::recursive_mutex m_outputLock{ };
    TiltedPhoques::Vector<std::pair<char, std::string>> m_outputLines{ };
    TiltedPhoques::Vector<std::string> m_consoleHistory{ };
    float m_outputNormalizedWidth{ 0.0f };
    int64_t m_consoleHistoryIndex{ 0 };
    bool m_newConsoleHistory{ true };
    bool m_outputShouldScroll{ true };
    bool m_outputScroll{ false };
    bool m_focusConsoleInput{ false };
    LuaVM& m_vm;
    D3D12& m_d3d12;

    std::string m_command;
    int m_commandLength{ 0 };
};