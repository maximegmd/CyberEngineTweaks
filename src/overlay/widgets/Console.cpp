#include <stdafx.h>

#include "Console.h"

#include <scripting/LuaVM.h>
#include <Utils.h>

Console::Console(LuaVM& aVm)
    : m_vm(aVm)
{
    auto consoleSink = CreateCustomSinkMT([this](const std::string& msg) { Log(msg); });
    consoleSink->set_pattern("%L;%v");
    spdlog::get("scripting")->sinks().emplace_back(std::move(consoleSink));
}

WidgetResult Console::OnEnable()
{
    m_focusConsoleInput = true;
    return WidgetResult::ENABLED;
}

WidgetResult Console::OnDisable()
{
    std::fill_n(m_Command, sizeof(m_Command), 0);
    return WidgetResult::DISABLED;
}

int Console::HandleConsoleHistory(ImGuiInputTextCallbackData* apData)
{
    auto* pConsole = static_cast<Console*>(apData->UserData);

    const std::string* pStr = nullptr;

    if (pConsole->m_newConsoleHistory)
    {
        pStr = &pConsole->m_consoleHistory[pConsole->m_consoleHistoryIndex];
    }
    else if (apData->EventKey == ImGuiKey_UpArrow && pConsole->m_consoleHistoryIndex > 0)
    {
        pConsole->m_consoleHistoryIndex--;

        pStr = &pConsole->m_consoleHistory[pConsole->m_consoleHistoryIndex];
    }
    else if (apData->EventKey == ImGuiKey_DownArrow && pConsole->m_consoleHistoryIndex + 1 < static_cast<int64_t>(pConsole->m_consoleHistory.size()))
    {
        pConsole->m_consoleHistoryIndex++;

        pStr = &pConsole->m_consoleHistory[pConsole->m_consoleHistoryIndex];
    }

    pConsole->m_newConsoleHistory = false;

    if (pStr)
    {
        std::memcpy(apData->Buf, pStr->c_str(), pStr->length() + 1);
        apData->BufDirty = true;
        apData->BufTextLen = static_cast<int>(pStr->length());
        apData->CursorPos = apData->BufTextLen;
    }

    return 0;
}

void Console::Update()
{
    const auto itemWidth = GetAlignedItemWidth(2);

    if (ImGui::Button("Clear output", ImVec2(itemWidth, 0)))
    {
        std::lock_guard _{ m_outputLock };
        m_outputLines.clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_outputShouldScroll);

    const auto& style = ImGui::GetStyle();
    const auto inputLineHeight = ImGui::GetTextLineHeight() + style.ItemInnerSpacing.y * 2;

    if (ImGui::ListBoxHeader("##ConsoleHeader", ImVec2(-1, -(inputLineHeight + style.ItemSpacing.y))))
    {
        std::lock_guard _{ m_outputLock };

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(m_outputLines.size()));
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                // TODO - use level to color output
                auto [level, item] = m_outputLines[i];
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
    ImGui::SetNextItemWidth(-FLT_MIN);
    const auto execute = ImGui::InputText("##InputCommand", m_Command, std::size(m_Command), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory,  &HandleConsoleHistory, this);
    ImGui::SetItemDefaultFocus();
    if (execute)
    {
        const auto consoleLogger = spdlog::get("scripting");
        consoleLogger->info("> {}", m_Command);

        m_consoleHistoryIndex = m_consoleHistory.size();
        m_consoleHistory.emplace_back(m_Command);
        m_newConsoleHistory = true;

        if (!m_vm.ExecuteLua(m_Command))
            consoleLogger->info("Command failed to execute!");

        std::fill_n(m_Command, sizeof(m_Command), 0);

        m_focusConsoleInput = true;
    }
}

void Console::Log(const std::string& acpText)
{
    assert(!acpText.empty());
    assert(acpText.size() >= 2);
    assert(acpText[1] == ';');

    std::lock_guard _{ m_outputLock };

    size_t first = 2;
    const size_t size = acpText.size();
    while (first < size)
    {
        // find_first_of \r or \n
        size_t second = std::string::npos;
        for (size_t i = first; i != size; ++i)
        {
            const auto ch = acpText[i];
            if (ch == '\r' || ch == '\n')
            {
                second = i;
                break;
            }
        }

        if (second == std::string_view::npos)
        {
            m_outputLines.emplace_back(acpText[0], acpText.substr(first));
            break;
        }

        if (first != second)
            m_outputLines.emplace_back(acpText[0], acpText.substr(first, second-first));

        first = second + 1;
        char ch = acpText[first];
        while (ch == '\r' || ch == '\n')
            ch = acpText[++first];
    }

    m_outputScroll = true;
}
