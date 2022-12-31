#include "stdafx.h"

#include "LogWindow.h"

#include <Utils.h>

LogWindow::LogWindow(const std::string& acpLoggerName)
    : m_loggerName(acpLoggerName)
{
    auto logSink = CreateCustomSinkMT([this](const std::string& msg) { Log(msg); });
    logSink->set_pattern("%L;%v");
    spdlog::get(m_loggerName)->sinks().emplace_back(std::move(logSink));
}

void LogWindow::Draw(const ImVec2& size)
{
    const auto itemWidth = GetAlignedItemWidth(2);

    if (ImGui::Button("Clear output", ImVec2(itemWidth, 0)))
    {
        m_normalizedWidth = -1.0f;
        std::lock_guard _{m_lock};
        m_nextIndexToCheck = 0;
        m_lines.clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_shouldScroll);

    const auto& style = ImGui::GetStyle();

    const auto frameId = ImGui::GetID(("##" + m_loggerName).c_str());
    if (ImGui::BeginChildFrame(frameId, size, ImGuiWindowFlags_HorizontalScrollbar))
    {
        std::lock_guard _{m_lock};

        if (!m_lines.empty() && (m_normalizedWidth < 0.0f || m_nextIndexToCheck < m_lines.size()))
        {
            for (size_t i = m_nextIndexToCheck; i < m_lines.size(); ++i)
            {
                auto& line = m_lines[i].second;
                m_normalizedWidth = std::max(m_normalizedWidth, ImGui::CalcTextSize(line.c_str()).x);
            }
            m_nextIndexToCheck = m_lines.size();
        }
        const auto listItemWidth = std::max(m_normalizedWidth + style.ItemInnerSpacing.x * 2, ImGui::GetContentRegionAvail().x);

        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(m_lines.size()));
        while (clipper.Step())
        {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
            {
                auto [level, item] = m_lines[i];

                switch (level)
                {
                case spdlog::level::level_enum::trace: ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.0f, 0.0f, 1.0f, 1.0f}); break;

                case spdlog::level::level_enum::debug: ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.0f, 1.0f, 0.0f, 1.0f}); break;

                case spdlog::level::level_enum::warn: ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 1.0f, 0.0f, 1.0f}); break;

                case spdlog::level::level_enum::err:
                case spdlog::level::level_enum::critical: ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 0.0f, 0.0f, 1.0f}); break;

                case spdlog::level::level_enum::info:
                default: ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));
                }

                ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

                ImGui::SetNextItemWidth(listItemWidth);

                ImGui::PushID(i);
                ImGui::InputText(("##" + item).c_str(), item.data(), item.size(), ImGuiInputTextFlags_ReadOnly);
                ImGui::PopID();

                ImGui::PopStyleVar(2);
                ImGui::PopStyleColor(4);
            }
        }

        if (m_scroll)
        {
            if (m_shouldScroll)
            {
                ImGui::SetScrollHereY();
                ImGui::SetScrollX(0.0f);
            }
            m_scroll = false;
        }
    }
    ImGui::EndChildFrame();
}

void LogWindow::Log(const std::string& acpText)
{
    assert(!acpText.empty());
    assert(acpText.size() >= 2);
    assert(acpText[1] == ';');

    spdlog::level::level_enum level = spdlog::level::level_enum::off;
    switch (acpText[0])
    {
    case 'T': // trace
        level = spdlog::level::level_enum::trace;
        break;
    case 'D': // debug
        level = spdlog::level::level_enum::debug;
        break;
    case 'I': // info
        level = spdlog::level::level_enum::info;
        break;
    case 'W': // warning
        level = spdlog::level::level_enum::warn;
        break;
    case 'E': // error
        level = spdlog::level::level_enum::err;
        break;
    case 'C': // critical
        level = spdlog::level::level_enum::critical;
        break;
    }
    assert(level != spdlog::level::level_enum::off);

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
            std::lock_guard _{m_lock};
            m_lines.emplace_back(level, std::move(text));
            break;
        }

        if (first != second)
        {
            auto text = acpText.substr(first, second - first);
            std::lock_guard _{m_lock};
            m_lines.emplace_back(level, std::move(text));
        }

        first = second + 1;
        char ch = acpText[first];
        while (ch == '\r' || ch == '\n')
            ch = acpText[++first];
    }

    std::lock_guard _{m_lock};
    m_scroll = true;
}
