#include <stdafx.h>

#include "Console.h"

#include <scripting/LuaVM.h>

void Console::OnEnable()
{
    m_focusConsoleInput = true;
}

void Console::OnDisable()
{
    
}

void Console::Update()
{
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

    auto& style = ImGui::GetStyle();
    m_InputLineHeight = ImGui::GetTextLineHeight() * m_CommandLines + style.ItemInnerSpacing.y * 2;
    
    if (ImGui::ListBoxHeader("##ConsoleHeader", ImVec2(-1, -(m_InputLineHeight + style.ItemSpacing.y))))
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

                    std::strncpy(m_Command, str.c_str(), sizeof(m_Command) - 1);
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
    const auto execute = ImGui::InputTextMultiline("##InputCommand", m_Command, std::size(m_Command), ImVec2(-1, m_InputLineHeight), ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_AllowTabInput| ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SetItemDefaultFocus();
    if (execute)
    {
        auto consoleLogger = spdlog::get("console");
        consoleLogger->info("> {}", m_Command);
        
        if (!LuaVM::Get().ExecuteLua(m_Command))
            consoleLogger->info("Command failed to execute!");

        if (m_inputClear)
        {
            std::memset(m_Command, 0, sizeof(m_Command));
            m_CommandLines = 1;
        }

        m_focusConsoleInput = true;
    }
    //if (ImGui::IsKeyDown(VK_CONTROL) && ImGui::IsKeyPressed(VK_RETURN, false))
    //{
    //    ++m_CommandLines;
    //}
}

void Console::Log(const std::string& acpText)
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

bool Console::GameLogEnabled() const
{
    return !m_disabledGameLog;
}
