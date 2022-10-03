#include <stdafx.h>

#include "Console.h"

#include "HelperWidgets.h"
#include "Utils.h"

#include <scripting/LuaVM.h>

Console::Console(LuaVM& aVm)
    : m_vm(aVm)
{
    const auto consoleSink = CreateCustomSinkST([this](const std::string& msg) { Log(msg); });
    consoleSink->set_pattern("%v");
    spdlog::get("scripting")->sinks().push_back(consoleSink);

    const auto gamelogSink = CreateCustomSinkST([this](const std::string& msg) { GameLog(msg); });
    gamelogSink->set_pattern("%v");
    spdlog::get("gamelog")->sinks().push_back(gamelogSink);
}

WidgetResult Console::OnEnable()
{
    m_focusConsoleInput = true;
    return WidgetResult::ENABLED;
}

WidgetResult Console::OnDisable()
{
    return WidgetResult::DISABLED;
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
        std::memcpy(apData->Buf, pStr->c_str(), pStr->length() + 1);
        apData->BufDirty = true;
        apData->BufTextLen = pStr->length();
        apData->CursorPos = apData->BufTextLen;
    }

    return 0;
}

void Console::Update()
{
    const auto itemWidth = HelperWidgets::GetAlignedItemWidth(5);

    ImGui::Checkbox("Clear input", &m_inputClear);
    ImGui::SameLine(itemWidth, ImGui::GetStyle().ItemSpacing.x);
    if (ImGui::Button("Clear output", ImVec2(itemWidth, 0)))
    {
        std::lock_guard _{ m_outputLock };
        m_outputLines.clear();
    }
    ImGui::SameLine(2 * itemWidth + 1 * ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.x);
    ImGui::Checkbox("Auto-scroll", &m_outputShouldScroll);
    ImGui::SameLine(3 * itemWidth + 2 * ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.x);
    ImGui::Checkbox("Show Game Log window", &m_showGameLog);
    ImGui::SameLine(4 * itemWidth + 3 * ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.x);
    if (ImGui::Button("Reload all mods", ImVec2(itemWidth, 0)))
        m_vm.ReloadAllMods();

    auto& style = ImGui::GetStyle();
    auto inputLineHeight = ImGui::GetTextLineHeight() + style.ItemInnerSpacing.y * 2;

    if (ImGui::ListBoxHeader("##ConsoleHeader", ImVec2(-1, -(inputLineHeight + style.ItemSpacing.y))))
    {
        std::lock_guard _{ m_outputLock };

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
    ImGui::SetNextItemWidth(-FLT_MIN);
    const auto execute = ImGui::InputText("##InputCommand", m_Command, std::size(m_Command),
                                          ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackHistory,  &HandleConsoleHistory, this);
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

    DrawGameLog();
}

void Console::Log(const std::string& acpText)
{
    std::lock_guard _{ m_outputLock };

    size_t first = 0;
    size_t size = acpText.size();
    while (first < size)
    {
        // find_first_of \r or \n
        size_t second = std::string::npos;
        for (size_t i = first; i != size; ++i)
        {
            char ch = acpText[i];
            if (ch == '\r' || ch == '\n')
            {
                second = i;
                break;
            }
        }

        if (second == std::string_view::npos)
        {
            m_outputLines.emplace_back(acpText.substr(first));
            break;
        }

        if (first != second)
            m_outputLines.emplace_back(acpText.substr(first, second-first));

        first = second + 1;
        char ch = acpText[first];
        while (ch == '\r' || ch == '\n')
            ch = acpText[++first];
    }

    m_outputScroll = true;
}

void Console::GameLog(const std::string& acpText)
{
    std::lock_guard _{ m_gamelogLock };

    size_t first = 0;
    size_t size = acpText.size();
    while (first < size)
    {
        // find_first_of \r or \n
        size_t second = std::string::npos;
        for (size_t i = first; i != size; ++i)
        {
            char ch = acpText[i];
            if (ch == '\r' || ch == '\n')
            {
                second = i;
                break;
            }
        }

        if (second == std::string_view::npos)
        {
            m_gamelogLines.emplace_back(acpText.substr(first));
            break;
        }

        if (first != second)
            m_gamelogLines.emplace_back(acpText.substr(first, second-first));

        first = second + 1;
        char ch = acpText[first];
        while (ch == '\r' || ch == '\n')
            ch = acpText[++first];
    }

    m_gamelogScroll = true;
}

void Console::DrawGameLog()
{
    if (m_showGameLog)
    {
        ImGui::Begin("Game Log", &m_showGameLog);

        const auto itemWidth = HelperWidgets::GetAlignedItemWidth(2);

        if (ImGui::Button("Clear output", ImVec2(itemWidth, 0)))
        {
            std::lock_guard _{ m_gamelogLock };
            m_gamelogLines.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_gamelogShouldScroll);

        if (ImGui::ListBoxHeader("##GameLogHeader", ImVec2(-1, -1)))
        {
            std::lock_guard _{ m_gamelogLock };

            ImGuiListClipper clipper;
            clipper.Begin(m_gamelogLines.size());
            while (clipper.Step())
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                {
                    auto& item = m_gamelogLines[i];
                    ImGui::PushID(i);
                    ImGui::Selectable(item.c_str());
                    ImGui::PopID();
                }

            if (m_gamelogScroll)
            {
                if (m_gamelogShouldScroll)
                    ImGui::SetScrollHereY();
                m_gamelogScroll = false;
            }

            ImGui::ListBoxFooter();
        }

        ImGui::End();
    }
}
