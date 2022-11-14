#include <stdafx.h>

#include "Console.h"

#include <scripting/LuaVM.h>
#include <Utils.h>

Console::Console(Options& aOptions, LuaVM& aVm)
    : LogWindow(aOptions, "Console", "scripting")
    , m_vm(aVm)
{
    m_command.resize(255);
}

WidgetResult Console::OnDisable()
{
    m_command.clear();
    m_command.resize(255);

    return Widget::OnDisable();
}

int Console::HandleConsoleHistory(ImGuiInputTextCallbackData* apData)
{
    auto* pConsole = static_cast<Console*>(apData->UserData);

    const std::string* pStr = nullptr;

    if (pConsole->m_newHistory)
    {
        pStr = &pConsole->m_history[pConsole->m_historyIndex];
    }
    else if (apData->EventKey == ImGuiKey_UpArrow && pConsole->m_historyIndex > 0)
    {
        pStr = &pConsole->m_history[--pConsole->m_historyIndex];
    }
    else if (apData->EventKey == ImGuiKey_DownArrow && pConsole->m_historyIndex + 1 < pConsole->m_history.size())
    {
        pStr = &pConsole->m_history[++pConsole->m_historyIndex];
    }

    pConsole->m_newHistory = false;

    if (pStr)
    {
        apData->DeleteChars(0, apData->BufTextLen);
        apData->InsertChars(0, pStr->c_str());
        apData->SelectAll();
    }

    return 0;
}

int Console::HandleConsoleResize(ImGuiInputTextCallbackData* apData)
{
    auto* pConsole = static_cast<Console*>(apData->UserData);

    if (apData->BufTextLen + 1 >= apData->BufSize)
    {
        pConsole->m_command.resize((apData->BufSize * 3) / 2);
        apData->Buf = pConsole->m_command.data();
    }
    pConsole->m_commandLength = apData->BufTextLen;

    return 0;
}

int Console::HandleConsole(ImGuiInputTextCallbackData* apData)
{
    if (apData->EventFlag & ImGuiInputTextFlags_CallbackHistory)
        return HandleConsoleHistory(apData);

    if (apData->EventFlag & ImGuiInputTextFlags_CallbackResize)
        return HandleConsoleResize(apData);

    return 0;
}

void Console::OnUpdate()
{
    const auto& style = ImGui::GetStyle();
    const auto inputLineHeight = ImGui::GetTextLineHeight() + style.ItemInnerSpacing.y * 2;
    DrawLog({-FLT_MIN, -(inputLineHeight + style.ItemSpacing.y)});

    ImGui::SetNextItemWidth(-FLT_MIN);
    constexpr auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackResize;
    const auto execute = ImGui::InputText("##InputCommand", m_command.data(), m_command.capacity(), flags, &HandleConsole, this);
    ImGui::SetItemDefaultFocus();
    if (execute)
    {
        m_command.resize(m_commandLength);

        // make sure to ignore trailing and beginning whitespaces
        trim(m_command);

        const auto consoleLogger = spdlog::get("scripting");
        consoleLogger->info("> {}", m_command);

        // execute command and record it to history if it is not empty
        if (!m_command.empty())
        {
            if (!m_vm.ExecuteLua(m_command))
                consoleLogger->info("Command failed to execute!");

            m_command.shrink_to_fit();
            m_historyIndex = m_history.size();
            auto& history = m_history.emplace_back();
            history.swap(m_command);
            m_newHistory = true;

            m_command.resize(255);
            m_commandLength = 0;
        }

        ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
    }
}
