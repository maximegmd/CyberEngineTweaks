#include <stdafx.h>

#include "Console.h"

#include <CET.h>
#include <scripting/LuaVM.h>
#include <Utils.h>

Console::Console(LuaVM& aVm, D3D12& aD3D12)
    : m_vm(aVm)
    , m_d3d12(aD3D12)
{
    auto consoleSink = CreateCustomSinkMT([this](const std::string& msg) { Log(msg); });
    consoleSink->set_pattern("%L;%v");
    spdlog::get("scripting")->sinks().emplace_back(std::move(consoleSink));

    m_command.resize(255);
}

WidgetResult Console::OnEnable()
{
    m_focusConsoleInput = true;
    return WidgetResult::ENABLED;
}

WidgetResult Console::OnDisable()
{
    m_command.clear();
    m_command.resize(255);
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
        pConsole->m_command.resize(pStr->length());
        pConsole->m_command.shrink_to_fit();
        pConsole->m_command = *pStr;

        apData->Buf = pConsole->m_command.data();
        apData->BufDirty = true;
        apData->BufTextLen = static_cast<int>(pStr->length());
        apData->CursorPos = apData->BufTextLen;
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

void Console::Update()
{
    const auto itemWidth = GetAlignedItemWidth(2);

    if (ImGui::Button("Clear output", ImVec2(itemWidth, 0)))
    {
        std::lock_guard _{ m_outputLock };
        m_outputNormalizedWidth = 0.0f;
        m_outputLines.clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_outputShouldScroll);

    const auto& style = ImGui::GetStyle();
    const auto inputLineHeight = ImGui::GetTextLineHeight() + style.ItemInnerSpacing.y * 2;

    if (ImGui::BeginChildFrame(ImGui::GetID("##ConsoleHeader"), ImVec2(-1, -(inputLineHeight + style.ItemSpacing.y)), ImGuiWindowFlags_HorizontalScrollbar))
    {
        std::lock_guard _{ m_outputLock };

        if (!m_outputLines.empty() && m_outputNormalizedWidth == 0.0f)
        {
            for (auto& line : m_outputLines | std::views::values)
                m_outputNormalizedWidth = std::max(m_outputNormalizedWidth, ImGui::CalcTextSize(line.c_str()).x);
        }
        const auto itemWidth = std::max(m_outputNormalizedWidth + style.ItemInnerSpacing.x * 2, ImGui::GetContentRegionAvail().x);

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(m_outputLines.size()));
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                // TODO - use level to color output
                auto [level, item] = m_outputLines[i];
                ImGui::PushID(i);
                ImGui::SetNextItemWidth(itemWidth);
                ImGui::InputText(("##" + item).c_str(), item.data(), item.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::PopID();
            }
        }

        if (m_outputScroll)
        {
            if (m_outputShouldScroll)
            {
                ImGui::SetScrollHereY();
                ImGui::SetScrollX(0.0f);
            }
            m_outputScroll = false;
        }
    }
    ImGui::EndChildFrame();

    if (m_focusConsoleInput)
    {
        ImGui::SetKeyboardFocusHere();
        m_focusConsoleInput = false;
    }
    ImGui::SetNextItemWidth(-FLT_MIN);
    constexpr auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackResize;
    const auto execute = ImGui::InputText("##InputCommand", m_command.data(), m_command.capacity(), flags, &HandleConsole, this);
    ImGui::SetItemDefaultFocus();
    if (execute)
    {
        const auto consoleLogger = spdlog::get("scripting");
        consoleLogger->info("> {}", m_command);

        m_consoleHistoryIndex = m_consoleHistory.size();
        auto& history = m_consoleHistory.emplace_back();
        history.swap(m_command);
        m_command.resize(255);
        m_newConsoleHistory = true;

        if (!m_vm.ExecuteLua(m_command))
            consoleLogger->info("Command failed to execute!");

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
            auto text = acpText.substr(first);
            if (m_d3d12.IsInitialized())
                m_outputNormalizedWidth = std::max(m_outputNormalizedWidth, ImGui::CalcTextSize(text.c_str()).x);
            m_outputLines.emplace_back(acpText[0], std::move(text));
            break;
        }

        if (first != second)
        {
            auto text = acpText.substr(first, second-first);
            if (m_d3d12.IsInitialized())
                m_outputNormalizedWidth = std::max(m_outputNormalizedWidth, ImGui::CalcTextSize(text.c_str()).x);
            m_outputLines.emplace_back(acpText[0], std::move(text));
        }

        first = second + 1;
        char ch = acpText[first];
        while (ch == '\r' || ch == '\n')
            ch = acpText[++first];
    }

    m_outputScroll = true;
}
