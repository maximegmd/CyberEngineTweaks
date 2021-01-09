#include <stdafx.h>

#include "Console.h"

#include <scripting/LuaVM.h>

void Console::Update()
{
    if (!IsEnabled())
        return;

    ImGui::Checkbox("Clear Input", &m_inputClear);
    ImGui::SameLine();
    if (ImGui::Button("Clear Output"))
    {
        std::lock_guard<std::recursive_mutex> _{ m_outputLock };
        m_outputLines.clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Scroll Output", &m_outputShouldScroll);
    ImGui::SameLine();
    ImGui::Checkbox("Disable Game Log", &m_disabledGameLog);

    static char command[0x10000] = { 0 };
    static int cmdLines = 1;
    static float inputLineHeight = -1.0f;
    auto& style = ImGui::GetStyle();
    inputLineHeight = ImGui::GetTextLineHeight() * cmdLines + style.ItemInnerSpacing.y * 2;
    
    if (ImGui::ListBoxHeader("##ConsoleHeader", ImVec2(-1, -(inputLineHeight + style.ItemSpacing.y))))
    {
        std::lock_guard<std::recursive_mutex> _{ m_outputLock };

        ImGuiListClipper clipper;
        clipper.Begin(m_outputLines.size());
        while (clipper.Step())
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i) 
            {
                auto& item = m_outputLines[i];
                ImGui::PushID(i);
                if (ImGui::Selectable(item.c_str()))
                {
                    auto str = item;
                    if (item[0] == '>' && item[1] == ' ')
                        str = str.substr(2);

                    std::strncpy(command, str.c_str(), sizeof(command) - 1);
                    m_focusConsoleInput = true;
                }
                ImGui::PopID();
            }

        if (m_outputScroll)
        {
            if (m_outputShouldScroll)
                ImGui::SetScrollHereY();
            m_outputScroll = false;
        }
        
        ImGui::ListBoxFooter();
    }
    
    if (m_focusConsoleInput)
    {
        ImGui::SetKeyboardFocusHere();
        m_focusConsoleInput = false;
    }
    const auto execute = ImGui::InputTextMultiline("##InputCommand", command, std::size(command), ImVec2(-1, inputLineHeight), ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_AllowTabInput| ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SetItemDefaultFocus();
    if (execute)
    {
        Logger::ToConsoleFmt("> {}", command);
        
        if (!LuaVM::Get().ExecuteLua(command))
            Logger::ToConsole("Command failed to execute!");

        if (m_inputClear)
        {
            std::memset(command, 0, sizeof(command));
            cmdLines = 1;
        }
    }
    if (ImGui::IsKeyDown(VK_CONTROL) && ImGui::IsKeyPressed(VK_RETURN, false))
    {
        ++cmdLines;
    }
}

void Console::Toggle()
{
    m_enabled = !m_enabled;
    m_focusConsoleInput = m_enabled;
}

bool Console::IsEnabled() const
{
    return m_enabled;
}

void Console::Log(std::string_view acpText)
{
    std::lock_guard<std::recursive_mutex> _{ m_outputLock };

    size_t first = 0;
    while (first < acpText.size())
    {
        const auto second = acpText.find_first_of('\n', first);

        if (second == std::string_view::npos)
        {
            m_outputLines.emplace_back(acpText.substr(first));
            break;
        }

        if (first != second)
            m_outputLines.emplace_back(acpText.substr(first, second-first));

        first = second + 1;
    }

    m_outputScroll = true;
}

void Console::GameLog(std::string_view acpText)
{
    if (!m_disabledGameLog)
        Logger::ToConsole(acpText);
}
