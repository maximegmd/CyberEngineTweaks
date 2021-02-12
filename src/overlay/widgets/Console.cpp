#include <stdafx.h>

#include "Console.h"
#include "Utils.h"

#include <scripting/LuaVM.h>

Console::Console(LuaVM& aVm)
    : m_vm(aVm)
{
    const auto consoleSink = CreateCustomSinkST([this](const std::string& msg) { Log(msg); });
    consoleSink->set_pattern("%v");

    spdlog::get("scripting")->sinks().push_back(consoleSink);
}

void Console::OnEnable()
{
    m_focusConsoleInput = true;
}

void Console::OnDisable()
{
}

int Console::HandleConsoleHistory(ImGuiInputTextCallbackData* apData)
{
    auto* pConsole = static_cast<Console*>(apData->UserData);

    std::string* pStr = nullptr;

    if (pConsole->m_newConsoleHistory)
    {
        pStr = &pConsole->m_consoleHistory[pConsole->m_consoleHistoryIndex];
    }
    else if (apData->EventKey == ImGuiKey_UpArrow && pConsole->m_consoleHistoryIndex > 0)
    {
        pConsole->m_consoleHistoryIndex--;

        pStr = &pConsole->m_consoleHistory[pConsole->m_consoleHistoryIndex];
    }
    else if (apData->EventKey == ImGuiKey_DownArrow && pConsole->m_consoleHistoryIndex + 1 < pConsole->m_consoleHistory.size())
    {
        pConsole->m_consoleHistoryIndex++;

        pStr = &pConsole->m_consoleHistory[pConsole->m_consoleHistoryIndex];
    }

    pConsole->m_newConsoleHistory = false;

    if (pStr)
    {
        std::memcpy(apData->Buf, pStr->c_str(), pStr->size());
        apData->BufDirty = true;
        apData->BufTextLen = pStr->size();
        apData->CursorPos = apData->BufTextLen;
    }

    return 0;
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
    ImGui::SameLine();
    if (ImGui::Button("Reload All Mods"))
        m_vm.ReloadAllMods();
        
    auto& style = ImGui::GetStyle();
    auto inputLineHeight = ImGui::GetTextLineHeight() + style.ItemInnerSpacing.y * 2;
    
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
    ImGui::PushItemWidth(-1);
    const auto execute = ImGui::InputText("##InputCommand", m_Command, std::size(m_Command),
                                          ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory,  &HandleConsoleHistory, this);
    ImGui::PopItemWidth();
    ImGui::SetItemDefaultFocus();
    if (execute)
    {
        auto consoleLogger = spdlog::get("scripting");
        consoleLogger->info("> {}", m_Command);

        m_consoleHistoryIndex = m_consoleHistory.size();
        m_consoleHistory.push_back(m_Command);
        m_newConsoleHistory = true;
        
        if (!m_vm.ExecuteLua(m_Command))
            consoleLogger->info("Command failed to execute!");

        if (m_inputClear)
        {
            std::memset(m_Command, 0, sizeof(m_Command));
        }

        m_focusConsoleInput = true;
    }
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
